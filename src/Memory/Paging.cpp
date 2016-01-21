///PML4 -> PDPT -> PD -> PT

#define CONVERTTOPML4(x) x << 39
#define CONVERTTOPDPT(x) x << 30
#define CONVERTTOPD(x) x << 21
#define CONVERTTOPT(x) x << 12

#define CONVERTFROMPML4(x) (x >> 39) & 0x1FF
#define CONVERTFROMPDPT(x) (x >> 30) & 0x1FF
#define CONVERTFROMPD(x) (x >> 21) & 0x1FF
#define CONVERTFROMPT(x) (x >> 12) & 0x1FF

#define TABLEADDRMASK 0x7FFFFFFFFF000

#define GETTABLEADDR(x) (long)x & (long)TABLEADDRMASK

#define ProcessMemStart 0x2000000

unsigned long CurrentSP, CurrentBP, CurrentPage;
unsigned long CurrentPhyAddr, CurrentVirtAddr;
void PageTableSetup(long* TableEntries)
{
	for(int x = 0; x < 512; x++)
	{
		TableEntries[x] = (long)0;
	}
} 

class PageFile
{
	public:
	long* Pages;
	
	PageFile();
	bool MapAddress(unsigned long, unsigned long);
	bool RawMapAddress(unsigned long, unsigned long);
	void Activate();
	unsigned long GetFreeAddress();
	void SetupStartMemory();
	void CreateTable(unsigned long);
	unsigned long FreeAll();
};

PageFile::PageFile()
{
	Pages = (long*)0x0;
}

PageFile Paging; //The 1:1 table

void PageFile::CreateTable(unsigned long VirtAddr)
{
	void* TempStackPointer = (TempStack + 0xFFF);
	CurrentVirtAddr = VirtAddr;
	asm volatile("MOV %%CR3, %0; MOV %%RSP, %1; MOV %%RBP, %2;" 
				: "=r"(CurrentPage), "=r"(CurrentSP), "=r"(CurrentBP));
	Paging.Activate();
	asm volatile("MOV %0, %%RSP; MOV %0, %%RBP" : : "r"(TempStackPointer));
	
	long* NewPML4;
	long* NewPDPT;
	long* NewPD;
	long* NewPT;
	
	unsigned char PML4Index = CONVERTFROMPML4(CurrentVirtAddr);
	unsigned short PDPTIndex = CONVERTFROMPDPT(CurrentVirtAddr);
	unsigned short PDIndex = CONVERTFROMPD(CurrentVirtAddr);
	long Address = GETTABLEADDR(CurrentVirtAddr);
	if(Pages == 0x0)
	{
		NewPML4 = (long*)PhysMemory.UseFreePhyAddr();
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		Pages = (long*)NewPML4;
		Pages[PML4Index] = (long)NewPDPT | 0x7;
		NewPDPT[PDPTIndex] = (long)NewPD | 0x7;
		NewPD[PDIndex] = (long)NewPT | 0x7;
		asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
		return;
	}
	
	if(Pages[PML4Index] == (long)0)
	{
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		Pages[PML4Index] = (long)NewPDPT | 0x7;
		NewPDPT[PDPTIndex] = (long)NewPD | 0x7;
		NewPD[PDIndex] = (long)NewPT | 0x7;
		asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
		return;
	}
	long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
	if(PDPTTable[PDPTIndex] == (long)0)
	{
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PDPTTable[PDPTIndex] = (long)NewPD | 0x7;
		NewPD[PDIndex] = (long)NewPT | 0x7;
		NewPD[PDIndex] = (long)NewPT | 0x7;
		asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
		return;
	}
	long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
	if(PDTable[PDIndex] == (long)0)
	{
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PDTable[PDIndex] = (long)NewPT | 0x7;
		asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
		return;
	}
	asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
	return;
}

bool PageFile::RawMapAddress(unsigned long PhyAddr, unsigned long VirtAddr)
{
	unsigned long AlignedPhyAddr = GETTABLEADDR(PhyAddr);
	unsigned char PML4Index = CONVERTFROMPML4(VirtAddr);
	
	long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);

	long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	
	long* PETable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
	unsigned short PTIndex = CONVERTFROMPT(VirtAddr);
	PETable[PTIndex] = (long)AlignedPhyAddr | 0x7;
	__asm__("invlpg (%0)" : : "r"((void*)PDTable));
	return true;
}

bool PageFile::MapAddress(unsigned long PhyAddr, unsigned long VirtAddr)
{
	CreateTable(VirtAddr);
	void* TempStackPointer = (TempStack + 0xFFF);
	CurrentPhyAddr = PhyAddr;
	CurrentVirtAddr = VirtAddr;
	asm volatile("MOV %%CR3, %0; MOV %%RSP, %1; MOV %%RBP, %2;" 
				: "=r"(CurrentPage), "=r"(CurrentSP), "=r"(CurrentBP));
	//if(!PhysMemory.UsePhyAddr(PhysMemory[PhysMemory.AddrToPos((void*)PhyAddr)]))
		//return false;
	Paging.Activate();
	asm volatile("MOV %0, %%RSP; MOV %0, %%RBP" : : "r"(TempStackPointer));
	
	if(Testing)
	{
		Serial.WriteString(0x1, "\r\n\r\nPhyAddr: ");
		Serial.WriteLongHex(0x1, (long)CurrentPhyAddr);
		Serial.WriteString(0x1, "\r\nVirtAddr: ");
		Serial.WriteLongHex(0x1, (long)CurrentVirtAddr);
	}
	
	unsigned long AlignedPhyAddr = GETTABLEADDR(CurrentPhyAddr);
	unsigned char PML4Index = CONVERTFROMPML4(CurrentVirtAddr);
	
	if(Testing)
	{
		Serial.WriteString(0x1, "\r\nPML4 Index: ");
		Serial.WriteLongHex(0x1, (long)PML4Index);
	}
	
	long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
	unsigned short PDPTIndex = CONVERTFROMPDPT(CurrentVirtAddr);

	if(Testing)
	{
		Serial.WriteString(0x1, "\r\nPDPTTable: ");
		Serial.WriteLongHex(0x1, (long)PDPTTable);
		Serial.WriteString(0x1, "\r\nPDPT Index: ");
		Serial.WriteLongHex(0x1, (long)PDPTIndex);
	}
	
	long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
	unsigned short PDIndex = CONVERTFROMPD(CurrentVirtAddr);
	
	if(Testing)
	{
		Serial.WriteString(0x1, "\r\nPDTable: ");
		Serial.WriteLongHex(0x1, (long)PDTable);
		Serial.WriteString(0x1, "\r\nPDIndex: ");
		Serial.WriteLongHex(0x1, (long)PDIndex);
	}
	
	long* PTTable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
	unsigned short PTIndex = CONVERTFROMPT(CurrentVirtAddr);

	if(Testing)
	{
		Serial.WriteString(0x1, "\r\nPTTable: ");
		Serial.WriteLongHex(0x1, (long)PTTable);
		Serial.WriteString(0x1, "\r\nPTIndex: ");
		Serial.WriteLongHex(0x1, (long)PTIndex);
	}
	
	PTTable[PTIndex] = (long)AlignedPhyAddr | 0x7;
	
	if(Testing)
	{
		Serial.WriteString(0x1, "\r\nPTTable[PTIndex]: ");
		Serial.WriteLongHex(0x1, (long)PTTable[PTIndex]);
	}
	__asm__("invlpg (%0)" : : "r"((void*)PDTable));
	asm volatile("MOV %0, %%CR3; MOV %1, %%RSP; MOV %2, %%RBP;" : : "r"(CurrentPage), "r"(CurrentSP), "r"(CurrentBP));
	return true;
}

void PageFile::Activate()
{
	asm volatile("MOV %0, %%CR3"
			: : "a"(Pages));
}

unsigned long PageFile::GetFreeAddress()
{
	for(long PML4Index = 0; PML4Index < 512; PML4Index++)
	{
		if(Pages[PML4Index] == (long)0)
			return CONVERTTOPML4(PML4Index);
		long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
		
		for(long PDPTIndex = 0; PDPTIndex < 512; PDPTIndex++)
		{
			if(PDPTTable[PDPTIndex] == (long)0)
				return CONVERTTOPML4(PML4Index) + CONVERTTOPDPT(PDPTIndex);
			long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
				
			for(long PDIndex = 0; PDIndex < 512; PDIndex++)
			{
				if(PDTable[PDIndex] == (long)0)
					return CONVERTTOPML4(PML4Index) + CONVERTTOPDPT(PDPTIndex) + CONVERTTOPD(PDIndex);
				long* PETable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
				
				for(long PTIndex = 0; PTIndex < 512; PTIndex++)
				{
					if(PETable[PTIndex] == 0x00)
						return (CONVERTTOPML4(PML4Index) + CONVERTTOPDPT(PDPTIndex) + CONVERTTOPD(PDIndex) + CONVERTTOPT(PTIndex));
				}
			}			
		}
	}
	return (long)0;
}

unsigned long PageFile::FreeAll()
{
	for(long PML4Index = 0; PML4Index < 512; PML4Index++)
	{
		if(Pages[PML4Index] != (long)0)
		{
			long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
			for(long PDPTIndex = 0; PDPTIndex < 512; PDPTIndex++)
			{
				if(PDPTTable[PDPTIndex] != (long)0)
				{
					long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
					for(long PDIndex = 0; PDIndex < 512; PDIndex++)
					{
						if(PDTable[PDIndex] != (long)0)
						{
							long* PETable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
							for(long PTIndex = 0; PTIndex < 512; PTIndex++)
							{
								if(PETable[PTIndex] != (long)0)
								{
									PETable[PTIndex] = 0x0;
									//PhysMemory.FreePhyAddr(PhysMemory[PhysMemory.AddrToPos((void*)(CONVERTTOPML4(PML4Index) + CONVERTTOPDPT(PDPTIndex)
									//					+ CONVERTTOPD(PDIndex) + CONVERTTOPT(PTIndex)))]);
								}
							}
						}
					}
				}			
			}
		}
	}
	return (long)0;
}

void PageFile::SetupStartMemory()
{
	for(unsigned long i = 0; i < ProcessMemStart; i += 0x1000)
	{
		MapAddress(i, i);
	}
}

long PD[] __attribute__((section (".PD")))
{
(long)0x87,
(long)0x200087,
(long)0x400087,
(long)0x600087,
(long)0x800087,
(long)0xA00087,
(long)0xC00087,
(long)0xE00087,
(long)0x1000087,
(long)0x1200087,
(long)0x1400087,
(long)0x1600087,
(long)0x1800087,
(long)0x1A00087,
(long)0x1C00087,
(long)0x1E00087,
(long)0x2000087,
(long)0x2200087,
(long)0x2400087,
(long)0x2600087,
(long)0x2800087,
(long)0x2A00087,
(long)0x2C00087,
(long)0x2E00087,
(long)0x3000087,
(long)0x3200087,
(long)0x3400087,
(long)0x3600087,
(long)0x3800087,
(long)0x3A00087,
};
long PDPT[] __attribute__((section (".PDPT")))
{
(long)&PD + (long)0x07
};
long PML4[] __attribute__((section (".PML4"))) =
{
(long)&PDPT + (long)0x7
};