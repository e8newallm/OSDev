char Input8(short PAddr)
{
	char ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

short Input16(short PAddr)
{
	short ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

int Input32(short PAddr)
{
	int ReturnValue;
	asm volatile("IN %1, %0"
				 : "=a"(ReturnValue) : "Nd"(PAddr));
	return ReturnValue;
}

void Output8(short PAddr, char Message)
{
	asm volatile("OUTB %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}

void Output16(short PAddr, short Message)
{
	asm volatile("OUTW %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}

void Output32(short PAddr, int Message)
{
	asm volatile("OUT %0, %1"
				 : : "a"(Message), "Nd"(PAddr));
}