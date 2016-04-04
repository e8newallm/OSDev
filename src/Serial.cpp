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
	unsigned short Port = GetPort(COMPort);
	while(PortAvail(Port) == 0);
	
	Output8(Port + SerialData, Character);
	//Unlock();
	return true;
}

bool SerialController::WriteString(short COMPort, char* String)
{
	//Lock();
	unsigned short Port = GetPort(COMPort);
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

SerialQueue::SerialQueue()
{
	SameCycle = true;
	Head = 0;
	Tail = 0;
}

void SerialQueue::WriteToLog(char* String)
{
	Lock();
	while(Head == Tail && !SameCycle)
	{
		Unlock();
		YieldCPU();
		Lock();
	}
	int i;
	for(i = 0; i < 256 && String[i] != (char)0; i++)
	{
		Queue[Head][i] = String[i];
		//Serial.WriteString(0x1, "\r\n\tQueue[Head][i]: ");
		//Serial.WriteLongHex(0x1, Queue[Head][i]);
	}
	Queue[Head][i] = (char)0;
	Head++;
	if(Head == SerialQueueSize)
	{
		Head = 0;
		SameCycle = false;
	}
	Unlock();
}

extern char* LongToString(long);

void SerialQueue::WriteToLog(long Value)
{
	char* Temp = LongToString(Value);
	WriteToLog(Temp);
	Free(Temp);
}

void SerialQueue::WriteToLog(const char* String)
{
	WriteToLog((char*)String);
}

void SerialQueue::ReadFromLog(char* Destination)
{
	Lock();
	while(Head == Tail && SameCycle)
	{
		Unlock();
		YieldCPU();
		Lock();
	}
	int i;
	for(i = 0; i < 256 && Queue[Tail][i] != (char)0; i++)
	{
		Destination[i] = Queue[Tail][i];
	}
	Destination[i] = Queue[Tail][i];
	Tail++;
	if(Tail == SerialQueueSize)
	{
		Tail = 0;
		SameCycle = true;
	}
	Unlock();
}

void WriteToLog(char* String)
{
	SerialLog.WriteToLog(String);
}
