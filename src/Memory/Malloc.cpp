#define MallocBucketLimit 0 //4kB or 4MB later

//Usage
//0 = Free, 1 = InUse, 254 = Start, 255 = End
void AddVMemory(unsigned long Size)
{
	int NewFrameCount = (Size / 0x1000) + 2;
	int Current = 0;
	MBlockHeader* Start = (CurrentProcess->OwnerProcess)->StartBlock;
	MBlockHeader* PrevEnd = (CurrentProcess->OwnerProcess)->EndBlock;
	while(Current < NewFrameCount)
	{
		(CurrentProcess->OwnerProcess)->EndBlock = (MBlockHeader*)(((long)(CurrentProcess->OwnerProcess)->EndBlock) + 0x1000);
		(CurrentProcess->OwnerProcess)->Page.MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(CurrentProcess->OwnerProcess)->EndBlock);
		Current++;
	}
	if(PrevEnd->PrevUsage == MBlockHeader_Free)
		PrevEnd = (MBlockHeader*)((long)PrevEnd - PrevEnd->PrevSize - sizeof(MBlockHeader));
	PrevEnd->NextSize = ((long)(CurrentProcess->OwnerProcess)->EndBlock - (long)PrevEnd) - sizeof(MBlockHeader);
	PrevEnd->NextUsage = MBlockHeader_Free;
	((CurrentProcess->OwnerProcess)->EndBlock)->PrevSize = ((long)(CurrentProcess->OwnerProcess)->EndBlock - (long)PrevEnd) - sizeof(MBlockHeader);
	((CurrentProcess->OwnerProcess)->EndBlock)->PrevUsage = MBlockHeader_Free;
	((CurrentProcess->OwnerProcess)->EndBlock)->NextUsage = MBlockHeader_End;
	
	MBlockHeader* Check = (MBlockHeader*)(CurrentProcess->OwnerProcess)->StartBlock;
	long Count = 0;
	
	return;
}

void* Malloc(unsigned long Size)
{
	//Serial.WriteString(0x1, "\r\nMalloc call");
	if(Size == 0)
		return (void*)0;
	if(Size > MallocBucketLimit) //If Size is too large to be used in the bucket memories
	{
		MBlockHeader* Check = (CurrentProcess->OwnerProcess)->StartBlock;
		//Serial.WriteString(0x1, "\r\nTest. PrevUsage: ");
		//Serial.WriteLongHex(0x1, Check->PrevUsage);
		while(1)
		{
			if(Check->NextUsage == MBlockHeader_Free)
			{
				//Serial.WriteString(0x1, "\r\nFree block. Check Size: ");
				//Serial.WriteLongHex(0x1, Check->NextSize);
				//Serial.WriteString(0x1, " Size: ");
				//Serial.WriteLongHex(0x1, Size);
			}
			if(Check->NextUsage == MBlockHeader_Free && Check->NextSize >= Size)
			{
				//Serial.WriteString(0x1, "\r\nBlock found");
				if(Check->NextSize > Size)
				{
					//Serial.WriteString(0x1, "\r\nBlock larger");
					Check->NextUsage = MBlockHeader_InUse;
					MBlockHeader* FinalBlock = (MBlockHeader*)((long)Check + Check->NextSize + sizeof(MBlockHeader));
					Check->NextSize = Size;
					MBlockHeader* NextBlock = (MBlockHeader*)((long)Check + Check->NextSize + sizeof(MBlockHeader));
					NextBlock->PrevSize = Size;
					NextBlock->PrevUsage = MBlockHeader_InUse;
					NextBlock->NextUsage = MBlockHeader_Free;
					NextBlock->NextSize = ((long)FinalBlock - (long)NextBlock - sizeof(MBlockHeader));
					FinalBlock->PrevSize = ((long)FinalBlock - (long)NextBlock - sizeof(MBlockHeader));
					return (void*)((long)Check + sizeof(MBlockHeader));
				}
				else if(Check->NextSize == Size)
				{
					//Serial.WriteString(0x1, "\r\nBlock exact size");
					Check->NextUsage = MBlockHeader_InUse;
					((MBlockHeader*)((long)Check + Check->NextSize + sizeof(MBlockHeader)))->PrevUsage = MBlockHeader_InUse;
					return (void*)((long)Check + sizeof(MBlockHeader));
				}
			}
			//Serial.WriteString(0x1, "\r\nNext block");
			if(Check->NextUsage == MBlockHeader_End)
			{
				//Serial.WriteString(0x1, "\r\nAllocation Needed");
				if(Check->PrevUsage == MBlockHeader_Free)
					Check = (MBlockHeader*)((long)Check - Check->PrevSize - sizeof(MBlockHeader));
				AddVMemory(Size);
			}
			else
			{
				//Serial.WriteString(0x1, "\r\nNo allocation Needed");
				Check = (MBlockHeader*)((long)Check + Check->NextSize + sizeof(MBlockHeader));
			}
		}
	}
}

//TODO: Add in code to cope with program being loaded in before heap

void Free(void* Pointer) 
{
	if(Pointer == (void*)0)
		return;
	MBlockHeader* FirstBlock = (MBlockHeader*)((long)Pointer - sizeof(MBlockHeader));
	MBlockHeader* LastBlock = (MBlockHeader*)((long)FirstBlock + FirstBlock->NextSize + sizeof(MBlockHeader));
	if(FirstBlock->PrevUsage == MBlockHeader_Free)
		FirstBlock = (MBlockHeader*)((long)FirstBlock - FirstBlock->PrevSize - sizeof(MBlockHeader));
	if(LastBlock->NextUsage == MBlockHeader_Free)
		LastBlock = (MBlockHeader*)((long)LastBlock + LastBlock->NextSize + sizeof(MBlockHeader));
	FirstBlock->NextSize = (long)LastBlock - (long)FirstBlock - sizeof(MBlockHeader);
	FirstBlock->NextUsage = MBlockHeader_Free;
	LastBlock->PrevSize = (long)LastBlock - (long)FirstBlock - sizeof(MBlockHeader);
	LastBlock->PrevUsage = MBlockHeader_Free;
}