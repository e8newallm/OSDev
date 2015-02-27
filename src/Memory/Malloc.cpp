//TODO: Add an option into Malloc for small allocations (Allocate a separate frame for variables of that size, and use a bitmap to track)

MemorySeg* FindFirstBlock(unsigned long Size)
{	//TODO: Improve the efficiency
	unsigned long SizeCheck = Size + sizeof(BlockHeader);
	for(long Pos = 0; PhysMemory[Pos]->Usage != MEMORYSEG_EOM; Pos++)
	{
		if(PhysMemory[Pos]->LargestBlock >= SizeCheck && PhysMemory[Pos]->Usage == MEMORYSEG_INUSE)
		{
			return (PhysMemory.PhyMemMap + Pos);
		}
	}
	return PhysMemory.EOM();
}

MemorySeg* AllocateFrame()
{
	MemorySeg* MMapPointer = PhysMemory.FindFreePhyAddr();
	if(MMapPointer->Usage == MEMORYSEG_EOM)
	{
		Kernel_Panic("Out of Memory!");
	}
	PhysMemory.UsePhyAddr(MMapPointer);
	return MMapPointer;
}

void* AllocateBlock(unsigned int Size, MemorySeg* MemorySegment)
{
	BlockHeader* FindBlock = (BlockHeader*)MemorySegment->BaseAddress;
	unsigned int SizeCheck = Size + sizeof(BlockHeader);
	while(!(FindBlock->Usage == BLOCKHEADER_FREE && FindBlock->Size >= SizeCheck))
	{
		FindBlock = (BlockHeader*)((long)FindBlock + FindBlock->Size + sizeof(BlockHeader));
	}
	unsigned int NextSize = FindBlock->Size - Size - sizeof(BlockHeader);
	FindBlock->Size = Size;
	FindBlock->Usage = BLOCKHEADER_INUSE;
	BlockHeader* NextBlock = (BlockHeader*)((long)FindBlock + Size + sizeof(BlockHeader));
	NextBlock->PrevSize = Size;
	NextBlock->PrevUsage = BLOCKHEADER_INUSE;
	NextBlock->Size = NextSize;
	NextBlock->Usage = BLOCKHEADER_FREE;
	NextBlock = (BlockHeader*)((long)NextBlock + NextSize + sizeof(BlockHeader));
	NextBlock->PrevSize = NextSize;
	NextBlock->PrevUsage = BLOCKHEADER_FREE;
	MemorySegment->LargestBlock = 0;
	for(BlockHeader* Largest = (BlockHeader*)MemorySegment->BaseAddress;
		(long)Largest < MemorySegment->BaseAddress + PhysMemory.MemorySegSize;
		Largest = (BlockHeader*)((long)Largest + Largest->Size + sizeof(BlockHeader)))
	{
		
		if(!Largest->Usage && Largest->Size > MemorySegment->LargestBlock)
		{
			MemorySegment->LargestBlock = Largest->Size;
		}
	}
	return (void*)((long)FindBlock + sizeof(BlockHeader));
}

void* malloc(unsigned int Size)
{
	if(Size + sizeof(BlockHeader) < PhysMemory.MemorySegSize)
	{
		MemorySeg* MemFrame = FindFirstBlock(Size);
		if(MemFrame->Usage == BLOCKHEADER_EOB)
		{
			MemFrame = AllocateFrame();
			BlockHeader* Block = (BlockHeader*)(MemFrame->BaseAddress);
			Block->PrevSize = 0;
			Block->PrevUsage = BLOCKHEADER_SOB;
			Block->Usage = BLOCKHEADER_INUSE;
			Block->Size = Size;
			BlockHeader* NextBlock = (BlockHeader*)((long)Block + Size + sizeof(BlockHeader));
			NextBlock->PrevSize = Size;
			NextBlock->PrevUsage = BLOCKHEADER_INUSE;
			NextBlock->Usage = BLOCKHEADER_FREE;
			NextBlock->Size = PhysMemory.MemorySegSize - (3*sizeof(BlockHeader)) - Size;
			BlockHeader* FinalBlock = (BlockHeader*)((long)(MemFrame->BaseAddress) + PhysMemory.MemorySegSize - sizeof(BlockHeader));
			FinalBlock->PrevSize = NextBlock->Size;
			FinalBlock->PrevUsage = BLOCKHEADER_FREE;
			FinalBlock->Usage = BLOCKHEADER_EOB;
			FinalBlock->Size = 0;
			MemFrame->LargestBlock = 0;
			for(BlockHeader* Largest = (BlockHeader*)MemFrame->BaseAddress;
				(unsigned long)Largest < (MemFrame->BaseAddress + PhysMemory.MemorySegSize);
				Largest = (BlockHeader*)((unsigned long)Largest + Largest->Size + sizeof(BlockHeader)))
			{
				if(!Largest->Usage && Largest->Size > MemFrame->LargestBlock)
				{
					MemFrame->LargestBlock = Largest->Size;
				}
			}
			MemFrame->LargestBlock = NextBlock->Size;
			return (void*)((long)Block + (long)sizeof(BlockHeader));
		}
		else
		{
			return AllocateBlock(Size, MemFrame);
		}
	}
}

void free(void* Data)
{
	BlockHeader* HeaderChange = (BlockHeader*)((long)Data - sizeof(BlockHeader));
	BlockHeader* SecondHeader = (BlockHeader*)((long)HeaderChange + HeaderChange->Size + sizeof(BlockHeader));
	if(HeaderChange->PrevUsage == BLOCKHEADER_FREE)
		HeaderChange = (BlockHeader*)((long) HeaderChange - sizeof(BlockHeader) - HeaderChange->PrevSize);
	if(SecondHeader->Usage == BLOCKHEADER_FREE)
		SecondHeader = (BlockHeader*)((long) SecondHeader + sizeof(BlockHeader) + SecondHeader->Size);
	long Size = ((long)SecondHeader - (long)HeaderChange - sizeof(BlockHeader));
	HeaderChange->Size = Size;
	HeaderChange->Usage = BLOCKHEADER_FREE;
	SecondHeader->PrevSize = Size;
	SecondHeader->PrevUsage = BLOCKHEADER_FREE;
	for(char* Pointer = (char*)((long)HeaderChange + sizeof(BlockHeader)); Pointer < (char*)SecondHeader; Pointer++)
	{
		*Pointer = 0;
	}
}