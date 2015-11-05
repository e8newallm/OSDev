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
	long* Pages = 0x0;
	
	
	bool MapAddress(unsigned long, unsigned long);
	bool RawMapAddress(unsigned long, unsigned long);
	void Activate();
	unsigned long GetFreeAddress();
	void SetupStartMemory();
	void CreateTable(unsigned long);
};

PageFile Paging;

void PageFile::CreateTable(unsigned long VirtAddr)
{
	unsigned long CurrentPage;
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	Paging.Activate();
	long* NewPML4;
	long* NewPDPT;
	long* NewPD;
	long* NewPT;
	
	unsigned char PML4Index = CONVERTFROMPML4(VirtAddr);
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	long Address = GETTABLEADDR(VirtAddr);
	if(Pages == 0x0)
	{
		NewPML4 = (long*)PhysMemory.UseFreePhyAddr();
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		Pages = (long*)NewPML4;
		Pages[PML4Index] = (long)NewPDPT | 0x3;
		NewPDPT[PDPTIndex] = (long)NewPD | 0x3;
		NewPD[PDIndex] = (long)NewPT | 0x3;
		asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
		return;
	}
	
	if(Pages[PML4Index] == (long)0)
	{
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		Pages[PML4Index] = (long)NewPDPT | 0x3;
		NewPDPT[PDPTIndex] = (long)NewPD | 0x3;
		NewPD[PDIndex] = (long)NewPT | 0x3;
		asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
		return;
	}
	long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
	if(PDPTTable[PDPTIndex] == (long)0)
	
	{
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PDPTTable[PDPTIndex] = (long)NewPD | 0x3;
		NewPD[PDIndex] = (long)NewPT | 0x3;
		NewPD[PDIndex] = (long)NewPT | 0x3;
		asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
	}
	long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
	if(PDTable[PDIndex] == (long)0)
	{
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PDTable[PDIndex] = (long)NewPT | 0x3;
		asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
		return;
	}
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
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
	PETable[PTIndex] = (long)AlignedPhyAddr | 0x83;
	__asm__("invlpg (%0)" : : "r"((void*)PDTable));
	return true;
}

bool PageFile::MapAddress(unsigned long PhyAddr, unsigned long VirtAddr)
{
	CreateTable(VirtAddr);
	unsigned long CurrentPage;
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	Paging.Activate();
	
	unsigned long AlignedPhyAddr = GETTABLEADDR(PhyAddr);
	unsigned char PML4Index = CONVERTFROMPML4(VirtAddr);

	long* PDPTTable = (long*)(GETTABLEADDR(Pages[PML4Index]));
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);

	long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	
	long* PTTable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
	unsigned short PTIndex = CONVERTFROMPT(VirtAddr);

	PTTable[PTIndex] = (long)AlignedPhyAddr | 0x83;
	__asm__("invlpg (%0)" : : "r"((void*)PDTable));
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
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

void PageFile::SetupStartMemory()
{
	for(unsigned long i = 0; i < ProcessMemStart; i += 0x1000)
	{
		MapAddress(i, i);
	}
}

long PD[] __attribute__((section (".PD")))
{
(long)0x83,
(long)0x200083,
(long)0x400083,
(long)0x600083,
(long)0x800083,
(long)0xA00083,
(long)0xC00083,
(long)0xE00083,
(long)0x1000083,
(long)0x1200083,
(long)0x1400083,
(long)0x1600083,
(long)0x1800083,
(long)0x1A00083,
(long)0x1C00083,
(long)0x1E00083,
(long)0x2000083,
(long)0x2200083,
(long)0x2400083,
(long)0x2600083,
(long)0x2800083,
(long)0x2A00083,
(long)0x2C00083,
(long)0x2E00083,
(long)0x3000083,
(long)0x3200083,
(long)0x3400083,
(long)0x3600083,
(long)0x3800083,
(long)0x3A00083,
};
long PDPT[] __attribute__((section (".PDPT")))
{
(long)&PD + (long)0x03
};
long PML4[] __attribute__((section (".PML4"))) =
{
(long)&PDPT + (long)0x3
};