
#define ALLOC_BLOCK_SIZE 20

String::String()
{
	Length = 0;
	AllocSize = 1;
	Address = (char*)Malloc(ALLOC_BLOCK_SIZE);
}

String::String(char* String)
{
	char* Pointer = String;
	Length = 0;
	while(*Pointer != (char)0)
	{
		Length++;
		Pointer++;
	}
	long Allocate = Length + (ALLOC_BLOCK_SIZE - (Length % ALLOC_BLOCK_SIZE));
	AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Address = (char*)Malloc(Allocate);
	for(long i = 0; i < Length; i++)
	{
		Address[i] = String[i];
	}
}

String::String(const char* String)
{
	char* Pointer = (char*)	String;
	Length = 0;
	while(*Pointer != (char)0)
	{
		Length++;
		Pointer++;
	}
	long Allocate = Length + (ALLOC_BLOCK_SIZE - (Length % ALLOC_BLOCK_SIZE));
	AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Address = (char*)Malloc(Allocate);
	for(long i = 0; i < Length; i++)
	{
		Address[i] = String[i];
	}
}

String::~String()
{
	Free(Address);
}

char& String::operator[](long Position)
{
	return Address[Position];
}

String String::operator=(String Equal)
{
	Free(Address);
	this->Length = Equal.Length;
	this->AllocSize = Equal.AllocSize;
	this->Address = (char*)Malloc(AllocSize * ALLOC_BLOCK_SIZE);
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
	long Allocate = Temp.Length + (ALLOC_BLOCK_SIZE - (Temp.Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)Malloc(Allocate);
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

String String::operator+(char* Add)
{
	char* Pointer = Add;
	String Temp = String();
	int AddLength = 0;
	while(*Pointer != (char)0)
	{
		AddLength++;
		Pointer++;
	}
	Temp.Length = Length + AddLength;
	long Allocate = Temp.Length + (ALLOC_BLOCK_SIZE - (Temp.Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)Malloc(Allocate);
	int Pos = 0;
	for(int i = 0; i < Length; i++)
	{
		Temp.Address[Pos] = Address[i];
		Pos++;
	}
	for(int i = 0; i < AddLength; i++)
	{
		Temp.Address[Pos] = Add[i];
		Pos++;
	}
	return Temp;
}

String String::operator+(const char* Add)
{
	return operator+((char*)Add);
}

String String::operator+(long Value)
{
	String Temp = String();
	Temp.Length = Length + LongDigits(Value);
	bool Neg = Value < 0;
	if(Neg)
	{
		Value = -Value;
		Temp.Length++;
	}
	long Allocate = Temp.Length + (ALLOC_BLOCK_SIZE - (Temp.Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)Malloc(Allocate);
	int Pos = 0;
	for(int i = 0; i < Length; i++)
	{
		Temp.Address[Pos] = Address[i];
		Pos++;
	}
	if(Neg)
	{
		Temp.Address[Pos] = '-';
		Pos++;
	}
	Temp.Address[Pos] = '0';
	for(long i = LongDigits(Value)-1; Value != 0 ; i--)
	{
		Temp.Address[Pos+i] = (Value % 10) + '0';
		Value /= 10;
	}
	return Temp;
}

String operator+(char* First, String Second)
{
	char* Pointer = First;
	String Temp = String();
	int FirstLength = 0;
	while(*Pointer != (char)0)
	{
		FirstLength++;
		Pointer++;
	}
	Temp.Length = Second.Length + FirstLength;
	long Allocate = Temp.Length + (ALLOC_BLOCK_SIZE - (Temp.Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)Malloc(Allocate);
	int Pos = 0;
	for(int i = 0; i < FirstLength; i++)
	{
		Temp.Address[Pos] = First[i];
		Pos++;
	}
	for(int i = 0; i < Second.Length; i++)
	{
		Temp.Address[Pos] = Second[i];
		Pos++;
	}
	return Temp;
}

String operator+(const char* First, String Second)
{
	return operator+((char*) First, Second);
}

String ValueToHexStr(long Value)
{
	String Temp = String();
	Temp.Length = LongDigitsHex(Value) + 2;
	long Allocate = Temp.Length + (ALLOC_BLOCK_SIZE - (Temp.Length % ALLOC_BLOCK_SIZE));
	Temp.AllocSize = Allocate / ALLOC_BLOCK_SIZE;
	Temp.Address = (char*)Malloc(Allocate);
	int Pos = 0;
	Temp.Address[0] = '0';
	Temp.Address[1] = 'x';
	Temp.Address[2] = '0';
	for(long i = Temp.Length-1; Value != 0 ; i--)
	{
		Temp.Address[Pos+i] = Hex[Value % 16];
		Value /= 16;
	}
	return Temp;
}
