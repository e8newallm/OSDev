extern SerialController Serial;

struct CPUIDdat
{
	bool PageSizeEx; //Checks if CR4.PSE can be set, enabling 4M-Byte pages in 32-bit paging
	bool PAEPaging; //Checks if CR4.PAE can be set
	bool GlobalPage; //Checks if CR4.PGE can be set, allowing global pages
	bool PageAttTable; //Checks if PAT is supported
	bool PageSizeEx36; //Checks if PSE-36 is supported, allowing physical addresses up to 40 bits
	bool ProcessContextID; //Checks if CR4.PCIDE can be set, allowing process-context identifiers
	bool SuperModeExePrevent; //Checks if CR4.SMEP can be set, allowing supervisor-mode execution prevention
	bool SuperModeAccessPrevent; //Checks if CR4.SMAP can be set, allowing supervisor-mode access prevention
	bool ExecuteDisable; //Checks if IA32_EFER.NXE can be set, allowing PAE paging and IA-32e paging to disable execute access to selected pages
	bool OneGBPages; //Checks if 1-GB pages are supported with IA-32e paging
	bool IA32eSupport; //Checks if IA32_EFER.LME can be set, allowing IA-32e paging
	long MaxPhyAddr; //Checks the max physical address width supported
	long MaxLinAddr; //Checks the max linear address width supported
};
	
struct gdt_entry_bits
{
	unsigned int limit_low:16;
	unsigned int base_low : 24;
     //attribute byte split into bitfields
	unsigned int accessed :1;
	unsigned int read_write :1; //readable for code, writable for data
	unsigned int conforming_expand_down :1; //conforming for code, expand down for data
	unsigned int code :1; //1 for code, 0 for data
	unsigned int always_1 :1; //should be 1 for everything but TSS and LDT
	unsigned int DPL :2; //priviledge level
	unsigned int present :1;
     //and now into granularity
	unsigned int limit_high :4;
	unsigned int available :1;
	unsigned int always_0 :1; //should always be 0
	unsigned int big :1; //32bit opcodes for code, unsigned int stack for data
	unsigned int gran :1; //1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high :8;
} __attribute__((packed));

struct TSS_entry_bits_64
{
	unsigned int limit_low:16;
	unsigned int base_low : 24;
    //attribute byte split into bitfields
	unsigned int Type :4;
	unsigned int always_0 :1; //should be 1 for everything but TSS and LDT
	unsigned int DPL :2; //priviledge level
	unsigned int present :1;
     //and now into granularity
	unsigned int limit_high :4;
	unsigned int available :1;
	unsigned int always_0_2 :2; //should always be 0
	unsigned int gran :1; //1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high :8;
	unsigned int Base_top;
	unsigned int Reserved;
} __attribute__((packed));

struct tss_entry
{
	int Reserved1;
	unsigned long RSP0;
	unsigned long RSP1;
	unsigned long RSP2;
	long Reserved2;
	unsigned long IST1;
	unsigned long IST2;
	unsigned long IST3;
	unsigned long IST4;
	unsigned long IST5;
	unsigned long IST6;
	unsigned long IST7;
	long Reserved3;
	int Reserved4;
} __attribute__((packed));
	
extern "C" void* memcpy(void* destination, const void* source, long num)
{
	for(long i = 0; i < num; i++)
		((char*)destination)[i] = ((char*)source)[i];
}

extern "C" void* memset(void* str, int c, long Size)
{
	for(long i = 0; i < Size; i++)
		*(((char*)str)+i) = (unsigned char)c;
}

void WriteToLog(char* String)
{
	SerialLog.WriteToLog(String);
}

typedef unsigned long size_t;

void* operator new (size_t size, void* ptr)
{
	return ptr;
}


int strcmp(const char *s1, const char *s2)
{
  int ret = 0;

  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) ++s1, ++s2;

  if (ret < 0)

    ret = -1;
  else if (ret > 0)

    ret = 1 ;

  return ret;
}


int strcmpl(const char *s1, const char *s2, int length)
{
  int ret = 0;
  length--;
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && length) ++s1, ++s2, --length;

  if (ret < 0)

    ret = -1;
  else if (ret > 0)

    ret = 1 ;

  return ret;
}
