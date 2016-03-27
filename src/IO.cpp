char Input8(unsigned short PAddr)
{
	char ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

short Input16(unsigned short PAddr)
{
	short ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

int Input32(unsigned short PAddr)
{
	int ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

void Output8(unsigned short PAddr, unsigned char Message)
{
	asm volatile("OUTB %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}

void Output16(unsigned short PAddr, unsigned short Message)
{
	asm volatile("OUTW %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}

void Output32(unsigned short PAddr, unsigned int Message)
{
	asm volatile("OUT %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}
