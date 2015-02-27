void Kernel_Panic(char* Reason)
{
	PrintString("\r\nKernel panic: ", 0x0A);
	PrintString(Reason, 0x0A);
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	while(1)
	{
		asm("nop");
	}
}

void Kernel_Panic(const char* Reason)
{
	Kernel_Panic((char*)Reason);
}