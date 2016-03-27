PageFile::PageFile()
{
	Pages = (long*)PhysMemory.UseFreePhyAddr();
}

void PageFile::CreateTable(unsigned long VirtAddr)
{
	long* NewPDPT;
	long* NewPD;
	long* NewPT;
	
	unsigned short PML4Index = CONVERTFROMPML4(VirtAddr);
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	/*if(Pages == 0x0)
	{
		NewPML4 = (long*)PhysMemory.UseFreePhyAddr();
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		Pages = (long*)NewPML4;
		PhysicalAccess(Pages)[PML4Index] = (long)NewPDPT | 0x7;
		PhysicalAccess(NewPDPT)[PDPTIndex] = (long)NewPD | 0x7;
		PhysicalAccess(NewPD)[PDIndex] = (long)NewPT | 0x7;
		return;
	}*/
	
	if(PhysicalAccess(Pages)[PML4Index] == (long)0)
	{
		NewPDPT = (long*)PhysMemory.UseFreePhyAddr();
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PhysicalAccess(Pages)[PML4Index] = (long)NewPDPT | 0x7;
		PhysicalAccess(NewPDPT)[PDPTIndex] = (long)NewPD | 0x7;
		PhysicalAccess(NewPD)[PDIndex] = (long)NewPT | 0x7;
		return;
	}
	long* PDPTTable = (long*)(GETTABLEADDR(PhysicalAccess(Pages)[PML4Index]));
	if(PhysicalAccess(PDPTTable)[PDPTIndex] == (long)0)
	{
		NewPD = (long*)PhysMemory.UseFreePhyAddr();
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PhysicalAccess(PDPTTable)[PDPTIndex] = (long)NewPD | 0x7;
		PhysicalAccess(NewPD)[PDIndex] = (long)NewPT | 0x7;
		return;
	}
	long* PDTable = (long*)(GETTABLEADDR(PhysicalAccess(PDPTTable)[PDPTIndex]));
	if(PhysicalAccess(PDTable)[PDIndex] == (long)0)
	{
		NewPT = (long*)PhysMemory.UseFreePhyAddr();
		PhysicalAccess(PDTable)[PDIndex] = (long)NewPT | 0x7;
		return;
	}
	return;
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
				if(PhysicalAccess(PDPTTable)[PDPTIndex] != (long)0)
				{
					long* PDTable = (long*)(GETTABLEADDR(PDPTTable[PDPTIndex]));
					for(long PDIndex = 0; PDIndex < 512; PDIndex++)
					{
						if(PhysicalAccess(PDTable)[PDIndex] != (long)0)
						{
							long* PETable = (long*)(GETTABLEADDR(PDTable[PDIndex]));
							for(long PTIndex = 0; PTIndex < 512; PTIndex++)
							{
								if(PhysicalAccess(PETable)[PTIndex] != (long)0)
								{
									PhysicalAccess(PETable)[PTIndex] = 0x0;
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

bool PageFile::MapAddress(unsigned long PhyAddr, unsigned long VirtAddr)
{
	CreateTable(VirtAddr);
	unsigned long AlignedPhyAddr = GETTABLEADDR(PhyAddr);
	unsigned short PML4Index = CONVERTFROMPML4(VirtAddr);

	long* PDPTTable = (long*)(GETTABLEADDR(PhysicalAccess(Pages)[PML4Index]));
	unsigned short PDPTIndex = CONVERTFROMPDPT(VirtAddr);
	
	long* PDTable = (long*)(GETTABLEADDR(PhysicalAccess(PDPTTable)[PDPTIndex]));
	unsigned short PDIndex = CONVERTFROMPD(VirtAddr);
	
	long* PTTable = (long*)(GETTABLEADDR(PhysicalAccess(PDTable)[PDIndex]));
	unsigned short PTIndex = CONVERTFROMPT(VirtAddr);
	
	PhysicalAccess(PTTable)[PTIndex] = (long)AlignedPhyAddr | 0x7;
	
	return true;
}

void PageFile::Activate()
{
	asm volatile("MOV %0, %%CR3"
			: : "a"(Pages));
}

void PageFile::SetupStartMemory()
{
	for(int i = 0; i < 512; i++)
		PhysicalAccess(Pages)[i] = PhysicalAccess(ModelPaging.Pages)[i];
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
