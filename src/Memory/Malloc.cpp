//TODO: Add an option into Malloc for small allocations (Allocate a separate frame for variables of that size, and use a bitmap to track)

MemorySeg* FindFirstBlock(unsigned long Size)
{	//TODO: Improve the efficiency
	MemorySeg* MMapPointer;
	unsigned long SizeCheck = Size + sizeof(BlockHeader);
	for(MMapPointer = PhyMemMap; MMapPointer->Usage != (unsigned char)0xFF; MMapPointer++)
	{
		if(MMapPointer->LargestBlock >= SizeCheck && MMapPointer->Usage == (unsigned char)0x01)
		{
			return MMapPointer;
		}
	}
	return MMapPointer;
}

MemorySeg* AllocateFrame()
{
	MemorySeg* MMapPointer = FindFreePhyAddr();
	if(MMapPointer->Usage == (unsigned char)0xFF)
	{
		Kernel_Panic("Out of Memory!");
	}
	UsePhyAddr(MMapPointer);
	return MMapPointer;
}

void* AllocateBlock(unsigned int Size, MemorySeg* MemorySegment)
{
	BlockHeader* FindBlock = (BlockHeader*)MemorySegment->BaseAddress;
	unsigned int SizeCheck = Size + sizeof(BlockHeader);
	while(!(FindBlock->Usage == (unsigned char)0x00 && FindBlock->Size >= SizeCheck))
	{
		FindBlock = (BlockHeader*)((long)FindBlock + FindBlock->Size + sizeof(BlockHeader));
	}
	unsigned int NextSize = FindBlock->Size - Size - sizeof(BlockHeader);
	FindBlock->Size = Size;
	FindBlock->Usage = 1;
	BlockHeader* NextBlock = (BlockHeader*)((long)FindBlock + Size + sizeof(BlockHeader));
	NextBlock->PrevSize = Size;
	NextBlock->PrevUsage = 1;
	NextBlock->Size = NextSize;
	NextBlock->Usage = 0;
	NextBlock = (BlockHeader*)((long)NextBlock + NextSize + sizeof(BlockHeader));
	NextBlock->PrevSize = NextSize;
	NextBlock->PrevUsage = 0;
	MemorySegment->LargestBlock = 0;
	for(BlockHeader* Largest = (BlockHeader*)MemorySegment->BaseAddress;
		(long)Largest < MemorySegment->BaseAddress + MemorySegSize;
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
	if(Size + sizeof(BlockHeader) < MemorySegSize)
	{
		MemorySeg* MemFrame = FindFirstBlock(Size);
		if(MemFrame->Usage == (unsigned char)0xFF)
		{
			MemFrame = AllocateFrame();
			BlockHeader* Block = (BlockHeader*)(MemFrame->BaseAddress);
			Block->PrevSize = 0;
			Block->PrevUsage = 0xFE;
			Block->Usage = 1;
			Block->Size = Size;
			BlockHeader* NextBlock = (BlockHeader*)((long)Block + Size + sizeof(BlockHeader));
			NextBlock->PrevSize = Size;
			NextBlock->PrevUsage = 1;
			NextBlock->Usage = 0;
			NextBlock->Size = MemorySegSize - (3*sizeof(BlockHeader)) - Size;
			BlockHeader* FinalBlock = (BlockHeader*)((long)(MemFrame->BaseAddress) + MemorySegSize - sizeof(BlockHeader));
			FinalBlock->PrevSize = NextBlock->Size;
			FinalBlock->PrevUsage = 0;
			FinalBlock->Usage = 0xFF;
			FinalBlock->Size = 0;
			MemFrame->LargestBlock = 0;
			for(BlockHeader* Largest = (BlockHeader*)MemFrame->BaseAddress;
				(unsigned long)Largest < (MemFrame->BaseAddress + MemorySegSize);
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

void FrameData(MemorySeg* TestData)
{
	BlockHeader* Pointer = (BlockHeader*)(TestData->BaseAddress);
	PrintString("\r\n\r\nStarting position: ", 0x0A);
	PrintString(LongToStringHex((long)Pointer), 0x0A);
	if(Pointer->PrevSize != 0 || Pointer->PrevUsage != 0xFE)
	{
		PrintString("\r\nCorrupt frame: Starting header's Prev data not correct!", 0x0A);
		PrintString("\r\nPrevUsage should be 0xFE, it is: ", 0x0A);
		PrintString(LongToStringHex(Pointer->PrevUsage), 0x0A);
		PrintString("\r\nPrevSize should be 0, it is: ", 0x0A);
		PrintString(LongToStringHex(Pointer->PrevSize), 0x0A);
		return;
	}
	else
	{
		PrintString("\r\n\r\nSize of block: ", 0x0A);
		PrintString(LongToStringHex(Pointer->Size), 0x0A);
		PrintString("\r\nUsage: ", 0x0A);
		PrintString(LongToStringHex(Pointer->Usage), 0x0A);
	}
	while(Pointer->Usage != 0xFF)
	{
		BlockHeader* PrevPointer = Pointer;
		Pointer = (BlockHeader*)((long)Pointer + Pointer->Size + sizeof(BlockHeader));
		if(Pointer-> PrevUsage != PrevPointer-> Usage || Pointer-> PrevSize != PrevPointer-> Size)
		{
			PrintString("\r\n\r\nCorrupt frame: Previous header different to footer!", 0x0A);
			PrintString("\r\nUsage:", 0x0A);
			PrintString(LongToStringHex(PrevPointer->Usage), 0x0A);
			PrintString("\r\nSize: ", 0x0A);
			PrintString(LongToStringHex(PrevPointer->Size), 0x0A);
			PrintString("\r\nPrevUsage:", 0x0A);
			PrintString(LongToStringHex(Pointer->PrevUsage), 0x0A);
			PrintString("\r\nPrevSize: ", 0x0A);
			PrintString(LongToStringHex(Pointer->PrevSize), 0x0A);
			PrintString("\r\nPrevPointerPos: ", 0x0A);
			PrintString(LongToStringHex((long)PrevPointer), 0x0A);
			PrintString("\r\nPointerPos: ", 0x0A);
			PrintString(LongToStringHex((long)Pointer), 0x0A);
			return;
		}
		else
		{
			PrintString("\r\n\r\nSize of block: ", 0x0A);
			PrintString(LongToStringHex(Pointer->Size), 0x0A);
			PrintString("\r\nUsage: ", 0x0A);
			PrintString(LongToStringHex(Pointer->Usage), 0x0A);
		}
	}
	PrintString("\r\n\r\nEnd position: ", 0x0A);
	PrintString(LongToStringHex((long)Pointer), 0x0A);
	return;
}

void free(void* Data)
{
	BlockHeader* HeaderChange = (BlockHeader*)((long)Data - sizeof(BlockHeader));
	BlockHeader* SecondHeader = (BlockHeader*)((long)HeaderChange + HeaderChange->Size + sizeof(BlockHeader));
	if(HeaderChange->PrevUsage == 0x0)
		HeaderChange = (BlockHeader*)((long) HeaderChange - sizeof(BlockHeader) - HeaderChange->PrevSize);
	if(SecondHeader->Usage == 0x0)
		SecondHeader = (BlockHeader*)((long) SecondHeader + sizeof(BlockHeader) + SecondHeader->Size);
	long Size = ((long)SecondHeader - (long)HeaderChange - sizeof(BlockHeader));
	HeaderChange->Size = Size;
	HeaderChange->Usage = 0;
	SecondHeader->PrevSize = Size;
	SecondHeader->PrevUsage = 0;
	for(char* Pointer = (char*)((long)HeaderChange + sizeof(BlockHeader)); Pointer < (char*)SecondHeader; Pointer++)
	{
		*Pointer = 0;
	}
}