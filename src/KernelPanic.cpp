void Kernel_Panic(char* Reason)
{
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Serial.WriteString(0x1, "\r\n\r\nKernel panic: ");
	Serial.WriteString(0x1, Reason);
	if(CurrentThread != 0)
	{
		Serial.WriteString(0x1, "\r\nOffending program: ");
		Serial.WriteString(0x1, (CurrentThread->OwnerProcess)->ProcessName);
	}
	asm("CLI; HLT");
}

void Kernel_Panic(const char* Reason)
{
	Kernel_Panic((char*)Reason);
}