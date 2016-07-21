char* MemoryMap::operator[](unsigned long Position)
{
	if(Position > Size)
		return (PhyMemMap+Size);
	return (PhyMemMap+Position);
}

char* MemoryMap::EOM()
{
	return (PhyMemMap+Size);
}

void MemoryMap::Initialise(multiboot_info_t* mbd, char* Start, unsigned long MemSegLength)
{
	memory_map_t* mmap = (memory_map*)((long)mbd->mmap_addr);
	unsigned long MemoryAddr = 0x0;
	PhyMemMap = Start;
	MemorySegSize = MemSegLength;
	char* CurrentMap = PhyMemMap;
	
	while((long)mmap < (long)mbd->mmap_addr + (long)mbd->mmap_length)
	{
		unsigned long BaseAddr = (((unsigned long)mmap->base_addr_high) << 32) + (unsigned long)mmap->base_addr_low;
		unsigned long Length = (((unsigned long)mmap->length_high) << 32) + (unsigned long)mmap->length_low;
		if(MemoryAddr < BaseAddr)
		{
			MemoryAddr = BaseAddr;
			if(MemoryAddr % MemorySegSize != 0)MemoryAddr += MemorySegSize - (MemoryAddr % MemorySegSize);
		}
		while(MemoryAddr + MemorySegSize < BaseAddr + Length)
		{
			if(mmap->type == 1)
			{
				*CurrentMap = MEMORYSEG_FREE;
			}
			else
			{
				*CurrentMap = MEMORYSEG_LOCKED;
			}
			if(MemoryAddr + MemorySegSize > BaseAddr + Length && *CurrentMap == MEMORYSEG_FREE)
			{
				memory_map_t* Pointer = (memory_map_t*)((long)mmap + mmap->size + sizeof(unsigned int));
				if(Pointer->type == 0)
				{
					*CurrentMap = MEMORYSEG_LOCKED;
				}
			}
			Size++;
			CurrentMap++;
			MemoryAddr += MemorySegSize;
		}
		mmap = (memory_map_t*)((long)mmap + (long)mmap->size + (long)sizeof(unsigned int));
	}
	*CurrentMap = MEMORYSEG_EOM;
	char* LoopChk = FindPhyAddr((long*)CurrentMap);
	for(char* Pointer = FindPhyAddr((long*)PhyMemMap); Pointer <= LoopChk; Pointer += 1)  //Setting memory map's memory blocks as in-use
	{
		UsePhyAddr(Pointer, MEMORYSEG_LOCKED);
	}
}

unsigned long MemoryMap::AddrToPos(void* Address)
{
	return (unsigned long)(((unsigned long)Address - ((unsigned long)Address % MemorySegSize))/MemorySegSize);
}

char* MemoryMap::FindPhyAddr(void* MemoryAddr)
{
	//TODO: Make more efficient
	if(AddrToPos(MemoryAddr) > Size)
		return (PhyMemMap+Size);
	return (PhyMemMap+AddrToPos(MemoryAddr));
}

char* MemoryMap::FindFreePhyAddr()
{
	//TODO: Make more efficient
	for(int Pos = 0; PhyMemMap[Pos] != MEMORYSEG_EOM; Pos++)
	{
		if(PhyMemMap[Pos] == MEMORYSEG_FREE)
		{
			return (PhyMemMap+Pos);
		}
	}
	return EOM();
}

void* MemoryMap::UseFreePhyAddr(char MemUsage = MEMORYSEG_INUSE)
{
	char* Block = FindFreePhyAddr();
	if(UsePhyAddr(Block, MemUsage) == 0)
	{
		return (void*)(((long)Block - (long)PhyMemMap) * 0x1000);
	}
	return (void*)0;
}

int MemoryMap::UsePhyAddr(char* MemoryAddr, char MemUsage = MEMORYSEG_INUSE)
{
	if(*MemoryAddr != MEMORYSEG_FREE) //Return the current usage if memory block isn't free
	{
		return *MemoryAddr;
	}
	*MemoryAddr = MemUsage;
	return 0x00; //Return 0 on success
}

int MemoryMap::FreePhyAddr(char* MemoryAddr)
{
	//TODO: Add check to make sure process isn't freeing another process' frame
	if(*MemoryAddr != MEMORYSEG_INUSE || *MemoryAddr != MEMORYSEG_LOCKED) //Return the current usage if memory block isn't in use
	{
		return *MemoryAddr;
	}
	*MemoryAddr = MEMORYSEG_FREE;
	char* Start = (char*)(((long)MemoryAddr - (long)PhyMemMap) * 0x1000);
	for(char* Zero = Start; Zero <= Start + 0xFFF; Zero++) // TODO: Replace with queue system that uses downtime to zero blocks
	{
		*Zero = (long)0x00;
	}
	return 0x00; //Return 0 on success
}
