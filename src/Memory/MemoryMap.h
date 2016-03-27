struct MemorySeg
{
	unsigned long BaseAddress;
	unsigned char Usage;
	// Usage = 00 : Free
	// Usage = 01 : In use
	// Usage = 02 : Locked
	// Usage = 03 : Needs zeroing to prepare for usage
	// Usage = FF : End of Map
};

#define MEMORYSEG_FREE 0x00
#define MEMORYSEG_INUSE 0x01
#define MEMORYSEG_LOCKED 0x02
#define MEMORYSEG_ZERO 0x03
#define MEMORYSEG_EOM 0xFF

class MemoryMap
{
	public:
	unsigned long MemorySegSize;
	MemorySeg* PhyMemMap;
	unsigned long Size;
	
	MemorySeg* FindPhyAddr(void*);
	MemorySeg* FindFreePhyAddr();
	int UsePhyAddr(MemorySeg*, char);
	void* UseFreePhyAddr(char);
	int FreePhyAddr(MemorySeg*);
	void Initialise(multiboot_info_t*, MemorySeg*, unsigned long);
	unsigned long AddrToPos(void*);
	MemorySeg* operator[](unsigned long);
	MemorySeg* EOM();
};

MemoryMap PhysMemory;
