#define ALLOC_BLOCK_SIZE 20

String::String()
{
	Length = 0;
	AllocSize = 1;
	Address = (char*)malloc(ALLOC_BLOCK_SIZE);
}

String::String(char* String)
{
	char* Pointer = String;
	PrintString("\r\n\r\nPointer = ", 0x0A);
	PrintString(LongToStringHex(this), 0x0A);
	PrintString("\r\nString = |", 0x0A);
	Length = 0;
	while(*Pointer != (char)0)
	{
		PrintChar(*Pointer, 0x0A);
		PrintChar('(', 0x0A);
		PrintString(LongToString(Length), 0x0A);
		PrintString(") ", 0x0A);
		Length++;
		Pointer++;
	}
	//PrintString("|\r\nLength = ", 0x0A);
	//PrintString(LongToString(Length), 0x0A);
	long Allocate = Length + (ALLOC_BLOCK_SIZE - (Length % ALLOC_BLOCK_SIZE));
	AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	//Address = (char*)malloc(Allocate);
	for(long i = 0; i < Length; i++)
	{
		Address[i] = String[i];
	}
}

char& String::operator[](long Position)
{
	return Address[Position];
}

String String::operator=(String Equal)
{
	free(Address);
	this->Length = Equal.Length;
	this->AllocSize = Equal.AllocSize;
	this->Address = (char*)malloc(AllocSize * ALLOC_BLOCK_SIZE);
	for(long i = 0; i < Length; i++)
	{
		this->Address[i] = Equal.Address[i];
	}
	return *this;
}

String String::operator+(String Add)
{
	String Temp = String();
	Temp.Length = Length + Add.Length;
	long Allocate = Length + (ALLOC_BLOCK_SIZE - (Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)malloc(Allocate);
	int Pos = 0;
	for(int i = 0; i < Length; i++)
	{
		Temp.Address[Pos] = Address[i];
		Pos++;
	}
	for(int i = 0; i < Add.Length; i++)
	{
		Temp.Address[Pos] = Add.Address[i];
		Pos++;
	}
	return Temp;
}