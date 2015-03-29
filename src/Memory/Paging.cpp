//PML4 -> PDPT -> PD -> PT

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
	void Initialise();
	bool MapAddress(unsigned long, unsigned long);
	void Activate();
	unsigned long GetFreeAddress();
};

bool PageFile::MapAddress(unsigned long PhyAddr, unsigned long VirtAddr)
{
	unsigned long AlignedPhyAddr = PhyAddr & 0xFFFFFFFFFFFFF000;
	unsigned char PML4Index = CONVERTFROMPML4(VirtAddr);
	if(Pages[PML4Index] == (long)0)
	{
		long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
		//Serial.WriteString(0x1, "\r\nNew paging table at: ");
		//Serial.WriteLongHex(0x1, (long)(NewTable));
		PageTableSetup(NewTable);
		Pages[PML4Index] = (long)NewTable | 3; //Temporary
	}
	
	long* PDPTTable = (long*)((long)Pages[PML4Index] & 0x7FFFFFFFFF000);
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);
	if(PDPTTable[PDPTIndex] == (long)0)
	{
		long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
		//Serial.WriteString(0x1, "\r\nNew paging table at: ");
		//Serial.WriteLongHex(0x1, (long)(NewTable));
		PageTableSetup(NewTable);
		PDPTTable[PDPTIndex] = (long)NewTable | 3; //Temporary
	}
	
	long* PDTable = (long*)((long)PDPTTable[PDPTIndex] & 0x7FFFFFFFFF000);
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	if(PDTable[PDIndex] == (long)0)
	{
		long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
		//Serial.WriteString(0x1, "\r\nNew paging table at: ");
		//Serial.WriteLongHex(0x1, (long)(NewTable));
		PageTableSetup(NewTable);
		PDTable[PDIndex] = (long)NewTable | 3; //Temporary
	}
	
	long* PETable = (long*)((long)PDTable[PDIndex] & 0x7FFFFFFFFF000);
	unsigned short PTIndex = CONVERTFROMPT(VirtAddr);
	PETable[PTIndex] = (long)AlignedPhyAddr | 0x83;
	
	//__asm__("invlpg PETable");
	return true;
}

void PageFile::Activate()
{
	asm volatile("MOV %0, %%CR3"
			: : "a"(Pages));
}

void PageFile::Initialise()
{
	Pages = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
	PageTableSetup(Pages);
}

unsigned long PageFile::GetFreeAddress()
{
	for(int PML4Index = 0; PML4Index < 512; PML4Index++)
	{
		if(Pages[PML4Index] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			PageTableSetup(NewTable);
			Pages[PML4Index] = (long)NewTable | 3; //Temporary
		}
		
		long* PDPTTable = (long*)((long)Pages[PML4Index] & 0x7FFFFFFFFF000);
		
		for(int PDPTIndex = 0; PDPTIndex < 512; PDPTIndex++)
		{
			if(PDPTTable[PDPTIndex] == (long)0)
			{
				long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
				PageTableSetup(NewTable);
				PDPTTable[PDPTIndex] = (long)NewTable | 3; //Temporary
			}	
			
			long* PDTable = (long*)((long)PDPTTable[PDPTIndex] & 0x7FFFFFFFFF000);
			
			for(int PDIndex = 0; PDIndex < 512; PDIndex++)
			{
				if(PDTable[PDIndex] == (long)0)
				{
					long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
					PageTableSetup(NewTable);
					PDTable[PDIndex] = (long)NewTable | 3; //Temporary
				}
				
				long* PETable = (long*)((long)PDTable[PDIndex] & 0x7FFFFFFFFF000);
				
				for(int PTIndex = 0; PTIndex < 512; PTIndex++)
				{
					if(PETable[PTIndex] == 0x00)
					{
						return (CONVERTTOPML4(PML4Index) + CONVERTTOPDPT(PDPTIndex) + CONVERTTOPD(PDIndex) + CONVERTTOPT(PTIndex));
					}
				}
			}			
		}
	}
	return (long)0;
}

long PDTemp[] __attribute__((section (".PD")))
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

long PDPTTemp[] __attribute__((section (".PDPT")))
{
	(long)&PDTemp + (long)0x03
};

long PML4Temp[] __attribute__((section (".PML4")))  = 
{
	(long)&PDPTTemp + (long)0x3
};