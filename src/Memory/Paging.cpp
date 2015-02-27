struct PageTable
{
	long Entries[512];
};

PageTable PDTemp __attribute__((section (".PD")))
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
};

PageTable PDPTTemp __attribute__((section (".PDPT")))
{
	(long)&PDTemp + (long)0x03
};

PageTable PML4Temp __attribute__((section (".PML4")))  = 
{
	(long)&PDPTTemp + (long)0x3
};