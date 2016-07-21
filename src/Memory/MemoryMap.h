//TODO: REPLACE WITH BETTER SYSTEM 

#define MEMORYSEG_FREE (char)0x00
#define MEMORYSEG_INUSE (char)0x01
#define MEMORYSEG_LOCKED (char)0x02
#define MEMORYSEG_ZERO (char)0x03
#define MEMORYSEG_EOM (char)0xFF

// Usage = 00 : Free
// Usage = 01 : In use
// Usage = 02 : Locked
// Usage = 03 : Needs zeroing to prepare for usage
// Usage = FF : End of Map

class MemoryMap
{
	public:
	unsigned long MemorySegSize;
	char* PhyMemMap;
	unsigned long Size;
	
	char* FindPhyAddr(void*);
	char* FindFreePhyAddr();
	int UsePhyAddr(char*, char);
	void* UseFreePhyAddr(char);
	int FreePhyAddr(char*);
	void Initialise(multiboot_info_t*, char*, unsigned long);
	unsigned long AddrToPos(void*);
	char* operator[](unsigned long);
	char* EOM();
};

MemoryMap PhysMemory;
