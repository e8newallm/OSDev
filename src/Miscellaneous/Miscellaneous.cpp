extern SerialController Serial;

void Kernel_Panic(char* Reason)
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Serial.WriteString(0x1, "\r\nStack");
	long Temp = 0;
	for(int i = 0; i < 15; i++)
	{
		Serial.WriteString(0x1, "\r\n");
		Serial.WriteLongHex(0x1, i);
		Serial.WriteString(0x1, ": ");
		asm volatile("POP %0"
		: "=a"(Temp));
		Serial.WriteLongHex(0x1, Temp);
	}
	Serial.WriteString(0x1, "\r\nTime");
	Serial.WriteLongHex(0x1, TimeSinceStart);
	Serial.WriteString(0x1, "\r\nKernel panic: ");
	Serial.WriteString(0x1, Reason);
	
	asm("CLI; HLT");
}

void Kernel_Panic(const char* Reason)
{
	Kernel_Panic((char*)Reason);
}

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
	
extern "C" void* memcpy(void* destination, const void* source, long num)
{
	for(long i = 0; i < num; i++)
		((char*)destination)[i] = ((char*)source)[i];
}
