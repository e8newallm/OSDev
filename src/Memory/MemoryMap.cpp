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

long MemorySegSize = 0x1000;
MemorySeg* PhyMemMap;
MemorySeg* PhyMemMapEnd;

MemorySeg* FindPhyAddr(long* MemoryAddr)
{
	//TODO: Make more efficient
	long* CheckAddr = (long*)((long)MemoryAddr - ((long)MemoryAddr % MemorySegSize));
	MemorySeg* CheckMemSeg;
	for(CheckMemSeg = PhyMemMap; CheckMemSeg->Usage != (char)0xFF; CheckMemSeg++)
	{

		if((long*)CheckMemSeg->BaseAddress == CheckAddr) return CheckMemSeg;
	}
	return CheckMemSeg;
}

MemorySeg* FindFreePhyAddr()
{
	//TODO: Make more efficient
	MemorySeg* CheckMemSeg;
	for(CheckMemSeg = PhyMemMap; CheckMemSeg->Usage != (char)0xFF; CheckMemSeg++)
	{
		if(CheckMemSeg->Usage == (char)0x00)
		{
			return CheckMemSeg;
		}
	}
	return CheckMemSeg;
}

int UsePhyAddr(MemorySeg* MemoryAddr, char MemUsage = 0x01)
{
	if(MemoryAddr < PhyMemMap || MemoryAddr > PhyMemMapEnd || ((long)MemoryAddr - (long)PhyMemMap) % sizeof(MemorySeg) != 0x00) //Return -1 if MemoryAddr isn't a physical memory map entry
	{
		return -1;
	}
	if(MemoryAddr->Usage != (char)0x00) //Return the current usage if memory block isn't free
	{
		return MemoryAddr->Usage;
	}
	MemoryAddr->Usage = MemUsage;
	MemoryAddr->LargestBlock = MemorySegSize;
	return 0x00; //Return 0 on success
}

int FreePhyAddr(MemorySeg* MemoryAddr)
{
	//TODO: Add check to make sure process isn't freeing another process' frame
	if(MemoryAddr < PhyMemMap || MemoryAddr > PhyMemMapEnd || ((long)MemoryAddr - (long)PhyMemMap) % sizeof(MemorySeg) != 0x00) //Return -1 if MemoryAddr isn't a physical memory map entry
	{
		return -1;
	}
	if(MemoryAddr->Usage != 0x01) //Return the current usage if memory block isn't in use
	{
		return MemoryAddr->Usage;
	}
	MemoryAddr->Usage = 0x00;
	MemoryAddr->LargestBlock = 0x000;
	for(char* Zero = (char*)MemoryAddr->BaseAddress; Zero <= (char*)MemoryAddr->BaseAddress + 0xFFF; Zero++) // TODO: Replace with queue system that uses downtime to zero blocks
	{
		*Zero = (long)0x00;
	}
	return 0x00; //Return 0 on success
}