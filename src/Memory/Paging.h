///PML4 -> PDPT -> PD -> PT

#define CONVERTTOPML4(x) (x << 39)
#define CONVERTTOPDPT(x) (x << 30)
#define CONVERTTOPD(x) (x << 21)
#define CONVERTTOPT(x) (x << 12)

#define CONVERTFROMPML4(x) ((x >> 39) & 0x1FF)
#define CONVERTFROMPDPT(x) ((x >> 30) & 0x1FF)
#define CONVERTFROMPD(x) ((x >> 21) & 0x1FF)
#define CONVERTFROMPT(x) ((x >> 12) & 0x1FF)

#define TABLEADDRMASK 0x7FFFFFFFFF000

#define GETTABLEADDR(x) (long)x & (long)TABLEADDRMASK

#define ProcessMemStart 0x2000000

#define PhysicalTable 0xFFFF800000000000
#define PhysicalAccess(x) ((long*)(PhysicalTable + (long)x))

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

PageFile ModelPaging; //The initial paging setup
