#define SerialData 0x0
#define SerialIntEnable 0x1
#define SerialIntID 0x2
#define SerialLineControl 0x3
#define SerialModemControl 0x4
#define SerialLineStatus 0x5
#define SerialModemStatus 0x6
#define SerialScratch 0x7

class SerialController
{
	public:
	short COM1, COM2, COM3, COM4;
	
	void Setup(char*);
	bool WriteChar(short, char);
	bool WriteString(short, char*);
	bool WriteString(short, const char*);
	bool WriteLongHex(short, long);
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
	short Port = GetPort(COMPort);
	while(PortAvail(Port) == 0);
	
	Output8(Port + SerialData, Character);
	return true;
}

bool SerialController::WriteString(short COMPort, char* String)
{
	for(int Pos = 0; String[Pos] != (char)0; Pos++)
	{
		WriteChar(COMPort, String[Pos]);
	}
	return true;
}

bool SerialController::WriteLongHex(short COMPort, long Value)
{
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
	for(Pos--; Pos >= 0; Pos--)
	{
		WriteChar(COMPort, Output[Pos]);
	}
	
	return true;
}

bool SerialController::WriteString(short COMPort, const char* String)
{
	return WriteString(COMPort, (char*)String);
}

SerialController Serial;