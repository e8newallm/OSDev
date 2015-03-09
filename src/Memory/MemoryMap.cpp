struct MemorySeg
{
	unsigned long BaseAddress;
	unsigned int LargestBlock;
	unsigned char Usage;
	// Usage = 00 : Free
	// Usage = 01 : In use
	// Usage = 02 : Locked
	// Usage = 03 : Needs zeroing to prepare for usage
	// Usage = FF : End of Map
};

const char MEMORYSEG_FREE = 0x00;
const char MEMORYSEG_INUSE = 0x01;
const char MEMORYSEG_LOCKED = 0x02;
const char MEMORYSEG_ZERO = 0x03;
const char MEMORYSEG_EOM = 0xFF;

struct BlockHeader
{
	unsigned int PrevSize;
	unsigned char PrevUsage;
	unsigned char Usage;
	//Usage = 00 : Free
	//Usage = 01 : In Use
	//Usage = FF : EoF
	//Usage = FE : SoF
	unsigned int Size;
};

const char BLOCKHEADER_FREE = 0x00;
const char BLOCKHEADER_INUSE = 0x01;
const char BLOCKHEADER_SOB = 0xFE;
const char BLOCKHEADER_EOB = 0xFF;

class MemoryMap
{
	public:
	unsigned long MemorySegSize;
	MemorySeg* PhyMemMap;
	unsigned long Size;
	
	MemorySeg* FindPhyAddr(void*);
	MemorySeg* FindFreePhyAddr();
	int UsePhyAddr(MemorySeg*, char);
	void* UseFreePhyAddr(char);
	int FreePhyAddr(MemorySeg*);
	void Initialise(multiboot_info_t*, MemorySeg*, long);
	long AddrToPos(void*);
	MemorySeg* operator[](unsigned long);
	MemorySeg* EOM();
};

MemorySeg* MemoryMap::operator[](unsigned long Position)
{
	if(Position > Size)
		return (PhyMemMap+Size);
	return (PhyMemMap+Position);
}

MemorySeg* MemoryMap::EOM()
{
	return (PhyMemMap+Size);
}

void MemoryMap::Initialise(multiboot_info_t* mbd, MemorySeg* Start, long MemSegLength)
{
	memory_map_t* mmap = (memory_map*)((long)mbd->mmap_addr);
	long MemoryAddr = 0x0;
	PhyMemMap = Start;
	MemorySegSize = MemSegLength;
	MemorySeg* CurrentMap = PhyMemMap;
	
	while((long)mmap < (long)mbd->mmap_addr + (long)mbd->mmap_length)
	{
		long BaseAddr = (((long)mmap->base_addr_high) << 32) + (long)mmap->base_addr_low;
		long Length = (((long)mmap->length_high) << 32) + (long)mmap->length_low;
		if(MemoryAddr < BaseAddr)
		{
			MemoryAddr = BaseAddr;
			if(MemoryAddr % MemorySegSize != 0)MemoryAddr += MemorySegSize - (MemoryAddr % MemorySegSize);
		}
		while(MemoryAddr + MemorySegSize < BaseAddr + Length)
		{
			CurrentMap->BaseAddress = MemoryAddr;
			if(mmap->type == 1)
			{
				CurrentMap->Usage = MEMORYSEG_FREE;
			}
			else
			{
				CurrentMap->Usage = MEMORYSEG_LOCKED;
			}
			if(MemoryAddr + MemorySegSize > BaseAddr + Length && CurrentMap->Usage == MEMORYSEG_FREE)
			{
				memory_map_t* Pointer = (memory_map_t*)((long)mmap + mmap->size + sizeof(unsigned int));
				if(Pointer->type == 0)
				{
					CurrentMap->Usage = MEMORYSEG_LOCKED;
				}
			}
			Size++;
			CurrentMap++;
			MemoryAddr += MemorySegSize;
		}
		mmap = (memory_map_t*)((long)mmap + (long)mmap->size + (long)sizeof(unsigned int));
	}
	CurrentMap->Usage = MEMORYSEG_EOM;
	MemorySeg* LoopChk = FindPhyAddr((long*)CurrentMap);
	for(MemorySeg* Pointer = FindPhyAddr((long*)PhyMemMap); Pointer <= LoopChk; Pointer += 1)  //Setting memory map's memory blocks as in-use
	{
		UsePhyAddr(Pointer, MEMORYSEG_LOCKED);
	}
}

long MemoryMap::AddrToPos(void* Address)
{
	return (long)(((long)Address - ((long)Address % MemorySegSize))/MemorySegSize);
}

MemorySeg* MemoryMap::FindPhyAddr(void* MemoryAddr)
{
	//TODO: Make more efficient
	if(AddrToPos(MemoryAddr) > Size)
		return (PhyMemMap+Size);
	return (PhyMemMap+AddrToPos(MemoryAddr));
}

MemorySeg* MemoryMap::FindFreePhyAddr()
{
	//TODO: Make more efficient
	for(int Pos = 0; PhyMemMap[Pos].Usage != MEMORYSEG_EOM; Pos++)
	{
		if(PhyMemMap[Pos].Usage == MEMORYSEG_FREE)
		{
			return (PhyMemMap+Pos);
		}
	}
	return (PhyMemMap+Size);
}

void* MemoryMap::UseFreePhyAddr(char MemUsage = MEMORYSEG_INUSE)
{
	MemorySeg* Block = FindFreePhyAddr();
	if(UsePhyAddr(Block, MemUsage) == 0)
	{
		return (void*)Block->BaseAddress;
	}
	return (void*)0;
}

int MemoryMap::UsePhyAddr(MemorySeg* MemoryAddr, char MemUsage = MEMORYSEG_INUSE)
{
	if(AddrToPos(MemoryAddr) >= Size || ((long)MemoryAddr - (long)PhyMemMap) % sizeof(MemorySeg) != 0x00) //Return -1 if MemoryAddr isn't a physical memory map entry
	{
		return -1;
	}
	if(MemoryAddr->Usage != MEMORYSEG_FREE) //Return the current usage if memory block isn't free
	{
		return MemoryAddr->Usage;
	}
	MemoryAddr->Usage = MemUsage;
	MemoryAddr->LargestBlock = MemorySegSize;
	return 0x00; //Return 0 on success
}

int MemoryMap::FreePhyAddr(MemorySeg* MemoryAddr)
{
	//TODO: Add check to make sure process isn't freeing another process' frame
	if(AddrToPos(MemoryAddr) > Size || ((long)MemoryAddr - (long)PhyMemMap) % sizeof(MemorySeg) != 0x00) //Return -1 if MemoryAddr isn't a physical memory map entry
	{
		return -1;
	}
	if(MemoryAddr->Usage != MEMORYSEG_INUSE || MemoryAddr->Usage != MEMORYSEG_LOCKED) //Return the current usage if memory block isn't in use
	{
		return MemoryAddr->Usage;
	}
	MemoryAddr->Usage = MEMORYSEG_FREE;
	MemoryAddr->LargestBlock = 0x000;
	for(char* Zero = (char*)MemoryAddr->BaseAddress; Zero <= (char*)MemoryAddr->BaseAddress + 0xFFF; Zero++) // TODO: Replace with queue system that uses downtime to zero blocks
	{
		*Zero = (long)0x00;
	}
	return 0x00; //Return 0 on success
}

extern MemoryMap PhysMemory;