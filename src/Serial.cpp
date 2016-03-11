#define SerialData 0x0
#define SerialIntEnable 0x1
#define SerialIntID 0x2
#define SerialLineControl 0x3
#define SerialModemControl 0x4
#define SerialLineStatus 0x5
#define SerialModemStatus 0x6
#define SerialScratch 0x7

class SerialController : protected CriticalRegion
{
	public:
	short COM1, COM2, COM3, COM4;
	
	void Setup(char*);
	bool WriteChar(short, char);
	bool WriteString(short, char*);
	bool WriteString(short, const char*);
	bool WriteLongHex(short, long);
	bool WriteLong(short, long);
	int PortAvail(short);
	short GetPort(int);
};

void SerialController::Setup(char* BDA)
{
	short* ComPorts = (short*)BDA;
	COM1 = ComPorts[0];
	COM2 = ComPorts[1];
	COM3 = ComPorts[2];
	COM4 = ComPorts[3];
	
	Output8(COM1 + SerialIntEnable, 0x00);
	Output8(COM1 + SerialLineControl, 0x80);
	Output8(COM1 + SerialData, 0x03);
	Output8(COM1 + SerialIntEnable, 0x00);
	Output8(COM1 + SerialLineControl, 0x03);
	Output8(COM1 + SerialIntID, 0xC7);
	Output8(COM1 + SerialModemControl, 0x0B);
}

int SerialController::PortAvail(short Port) {
   return Input8(Port + SerialLineStatus) & 0x20;
}

short SerialController::GetPort(int Port)
{
	switch(Port)
	{
		case 1:
			return COM1;
		case 2:
			return COM2;
		case 3:
			return COM3;
		case 4:
			return COM4;
		default:
			return COM1;
	}
}

bool SerialController::WriteChar(short COMPort, char Character)
{
	//Lock();
	//return true;
	short Port = GetPort(COMPort);
	while(PortAvail(Port) == 0);
	
	Output8(Port + SerialData, Character);
	//Unlock();
	return true;
}

bool SerialController::WriteString(short COMPort, char* String)
{
	//Lock();
	short Port = GetPort(COMPort);
	for(int Pos = 0; String[Pos] != (char)0; Pos++)
	{
		while(PortAvail(Port) == 0);
		Output8(Port + SerialData, String[Pos]);
	}
	//Unlock();
	return true;
}

bool SerialController::WriteLongHex(short COMPort, long Value)
{
	//Lock();
	char Output[16];
	int Pos = 0;
	unsigned long Temp = Value;
	if(Value == 0)
	{
		WriteChar(COMPort, '0');
		WriteChar(COMPort, 'x');
		WriteChar(COMPort, '0');
		return true;
	}
	while(Temp != 0)
	{
		Output[Pos] = Hex[Temp % 16];
		Temp /= 16;
		Pos++;
	}
	
	WriteChar(COMPort, '0');
	WriteChar(COMPort, 'x');
	short Port = GetPort(COMPort);
	for(Pos--; Pos >= 0; Pos--)
	{
		while(PortAvail(Port) == 0);
		Output8(Port + SerialData, Output[Pos]);
	}
	//Unlock();
	return true;
}

bool SerialController::WriteLong(short COMPort, long Value)
{
	//Lock();
	char Output[10];
	int Pos = 0;
	unsigned long Temp = Value;
	if(Value == 0)
	{
		WriteChar(COMPort, '0');
		return true;
	}
	while(Temp != 0)
	{
		Output[Pos] = Dec[Temp % 10];
		Temp /= 10;
		Pos++;
	}
	
	short Port = GetPort(COMPort);
	for(Pos--; Pos >= 0; Pos--)
	{
		while(PortAvail(Port) == 0);
		Output8(Port + SerialData, Output[Pos]);
	}
	//Unlock();
	return true;
}

bool SerialController::WriteString(short COMPort, const char* String)
{
	return WriteString(COMPort, (char*)String);
}

SerialController Serial;

void YieldCPU();
#define SerialQueueSize 30

class SerialQueue : protected CriticalRegion
{
	private:
	volatile char Queue[SerialQueueSize][256];
	char Position = 0;
	public:
	void WriteToLog(char*);
	void WriteToLog(const char*);
	void WriteToLog(long);
	void ReadFromLog(char*);
};

void SerialQueue::WriteToLog(char* String)
{
	Lock();
	while(Position >= SerialQueueSize)
	{
		Unlock();
		YieldCPU();
		Lock();
	}
	int i;
	for(i = 0; i < 256 && String[i] != (char)0; i++)
		Queue[Position][i] = String[i];
	Queue[Position][i] = (char)0;
	Position++;
	Unlock();
}

void SerialQueue::WriteToLog(long Value)
{
	Lock();
	while(Position >= SerialQueueSize)
	{
		Unlock();
		YieldCPU();
		Lock();
	}
	int Pos = 2;
	Queue[Position][0] = '0';
	Queue[Position][1] = 'x';
	int bits = 60;
	bool HighDigit = false;
	while(bits >= 0)
	{
		char Digit = (Value>>bits)&0xF;
		if(HighDigit == true || Digit != 0 || bits == 0)
		{
			Queue[Position][Pos] = Hex[Digit];
			Pos++;
			HighDigit = true;
		}
		bits -= 4;
	}
	Queue[Position][Pos] = (char)0;
	Position++;
	Unlock();
}

void SerialQueue::WriteToLog(const char* String)
{
	WriteToLog((char*)String);
}

void SerialQueue::ReadFromLog(char* Destination)
{
	Lock();
	while(Position == 0)
	{
		Unlock();
		YieldCPU();
		Lock();
	}
	Position--;
	int i;
	for(i = 0; i < 256 && Queue[Position][i] != (char)0; i++)
	{
		Destination[i] = Queue[Position][i];
	}
	Destination[i] = Queue[Position][i];
	Unlock();
}

SerialQueue SerialLog;
