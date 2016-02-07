/////////////////////////LIBRARY TOOLS////////////////////////////////////////////

__attribute__((noinline)) int CreateProcess(void* Main, const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(0x0), "b"(Main), "c"(Name));
	return ProcessID;
}

/////////////////////////TEST PROGRAMS////////////////////////////////////////////

__attribute__((noinline)) volatile void Graphics()
{
	while(1)
		YieldCPU();
	/*for(unsigned long i = 0; i <= GUI.BytesPerLine * GUI.Height; i += 0x1000)
	{
		(CurrentProcess->Page)->MapAddress(((unsigned long)GUI.FrameAddress + i), ((long)(CurrentProcess->OwnerProcess)->MemStart) + i);
	}
	//Serial.WriteString(0x1, "\r\nDone");
	GUI.FrameAddress = (unsigned char*)((long)(CurrentProcess->OwnerProcess)->MemStart);
	char shade = 255;
	char* MainFrame = (char*)GUI.FrameAddress;
	char* NewFrame = (char*)GUI.SecondFrameAddress;
	//Serial.WriteString(0x1, "\r\nWhile(1)");
	while(1)
	{
		for(int i = 0; i < GUI.Height; i++)
		{
			for(int j = 0; j < (GUI.Width*GUI.Depth); j++)
			{
				int pos = (i * GUI.BytesPerLine) + j;
				MainFrame[pos] = NewFrame[pos];
			}
		}
		YieldCPU();
	}*/
}

__attribute__((noinline)) volatile void SerialWrite()
{
	char NextLog[256];
	while(1)
	{
		SerialLog.ReadFromLog(NextLog);
		Serial.WriteString(0x1, "\r\nWriting to log: ");
		Serial.WriteString(0x1, NextLog);
	}
}

__attribute__((noinline)) volatile void TestProcess()
{
	//Serial.WriteString(0x1, "\r\nGraphics Loaded!");
	int x = 214, y = 532;
	int xVel = 15, yVel = 18;
	long Width = GUI.Width, Height = GUI.Height;
	long Time = TimeSinceStart;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		GUI.DrawPixel(x, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x, y+1, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y+1, (char)(128), (char)(50), (char)(75));
		//while(TimeSinceStart - Time < 500)
		//{
			YieldCPU();
		//}
		Time = TimeSinceStart;
		GUI.DrawPixel(x, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x, y+1, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y+1, (char)(0), (char)(0), (char)(0));
		//DrawString("Test string", 11, x, y);
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x > GUI.Width)
		{
			x = GUI.Width;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y > GUI.Height)
		{
			y = GUI.Height;
			yVel = -yVel;
		}
	}
}

__attribute__((noinline)) volatile void TestProcessTwo()
{
	//Serial.WriteString(0x1, "\r\nGraphics Loaded!");
	int x = 214, y = 532;
	int xVel = -15, yVel = -18;
	long Width = GUI.Width, Height = GUI.Height;
	long Time = TimeSinceStart;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		GUI.DrawPixel(x, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x, y+1, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y+1, (char)(128), (char)(50), (char)(75));
		//while(TimeSinceStart - Time < 500)
		//{
			YieldCPU();
		//}
		Time = TimeSinceStart;
		GUI.DrawPixel(x, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x, y+1, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y+1, (char)(0), (char)(0), (char)(0));
		//DrawString("Test string", 11, x, y);
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x > GUI.Width)
		{
			x = GUI.Width;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y > GUI.Height)
		{
			y = GUI.Height;
			yVel = -yVel;
		}
	}
}

__attribute__((noinline)) volatile void SystemIdle()
{
	Serial.WriteString(0x1, "\r\nTesting TSS0: ");
	Serial.WriteLongHex(0x1, TSS.RSP0);
	Serial.WriteString(0x1, "\r\n Stack: ");
	Serial.WriteLongHex(0x1, GetProcess(0)->GetThread(0)->TSSRSP);
	
	//int Test = CreateProcess((void*)&TestProcess, "Test Program");
	//GetProcess(Test)->Start();
	//SerialLog.WriteToLog("TEST SHIT");
	while(1)
	{
		YieldCPU();
		SerialLog.WriteToLog("\r\nTesting");
	}
}