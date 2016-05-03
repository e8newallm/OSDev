/////////////////////////TEST PROGRAMS////////////////////////////////////////////
///////TODO: COMPILE SEPARATELY AND LOAD USING GRUB MODULES///////////////////////

__attribute__((noinline)) void Graphics()
{
	long* MainFrame = (long*)ProcessMemStart;
	long* NewFrame = (long*)0xA00000;
	long Size = (GUI.Height * GUI.BytesPerLine)/8;
	MapVideoMemory((void*)ProcessMemStart);
	while(1)
	{
		for(long i = 0; i < Size; i++)
		{
			(MainFrame)[i] = (NewFrame)[i];
		}
		YieldCPU();
	}
}

__attribute__((noinline)) void SerialWrite()
{
	char NextLog[256];
	while(1)
	{
		SerialLog.ReadFromLog(NextLog);
		Serial.WriteString(0x1, NextLog);
	}
}

__attribute__((noinline)) void TaskManager()
{
	Draw Window = Draw(0, 0, GUI.Width/2, GUI.Height*3/4 + 7, GUI.Width, GUI.Height);
	while(1)
	{
		Window.DrawRect(3, 3, (GUI.Width/2) - 6, GUI.Height*3/4 + 4, 255, 255, 255);
		Window.DrawRect(5, 5, (GUI.Width/2) - 10, GUI.Height*3/4, 128, 128, 128);
		Window.DrawString("Task Manager", 10, 10);
		int y = 25;
		int CurProcessID = 0;
		Process* CurProcess = GetProcess(0);
		while(CurProcess->Available == false)
		{
			Window.DrawString(CurProcess->ProcessName, 19, y);
			y += 15;
			CurProcessID++;
			CurProcess = GetProcess(CurProcessID);
		}
		y+= 15;
		Window.DrawString("Time: ", 10, y);
		char* Temp = LongToString(GetMilli());
		Window.DrawString(Temp, 64, y);
		Free(Temp);
		Window.Update();
		//SerialLog.WriteToLog("DRAWING COMPLETE");
		YieldCPU();
	}
}

long Timer = 0;
Process* ProcessCheck;

__attribute__((noinline)) void TestAutomation()
{
	SerialLog.WriteToLog("\r\nStarting Testing");
	SerialLog.WriteToLog("\r\nTest 1: Pure CPU usage");
	Timer = GetMilli();
	int ID = CreateProcess(&TestOne, "Test One - Pure CPU", 1);
	ProcessCheck = GetProcess(ID);
	ProcessCheck->Start();
	WaitForThread(ID, 0);
	Timer = GetMilli() - Timer;
	SerialLog.WriteToLog("\r\nTest 1 result: ");
	SerialLog.WriteToLog(Timer);
	SerialLog.WriteToLog("\r\n\r\n");
	
	SerialLog.WriteToLog("\r\nTest 2: Graphics\r\n");
	Timer = GetMilli();
	ID = CreateProcess(&TestTwo, "Test Two - Graphics draw", 1);
	ProcessCheck = GetProcess(ID);
	ProcessCheck->Start();
	WaitForThread(ID, 0);
	Timer = GetMilli() - Timer;
	SerialLog.WriteToLog("\r\nTest 2 result: ");
	SerialLog.WriteToLog(Timer);
	SerialLog.WriteToLog("\r\n\r\n");
	
	SerialLog.WriteToLog("\r\nTest 3: Multithreaded CPU usage");
	Timer = GetMilli();
	ID = CreateProcess(&TestThree, "Test Three - Multithreaded CPU", 1);
	ProcessCheck = GetProcess(ID);
	ProcessCheck->Start();
	WaitForThread(ID, 0);
	Timer = GetMilli() - Timer;
	SerialLog.WriteToLog("\r\nTest 3 result: ");
	SerialLog.WriteToLog(Timer);
	SerialLog.WriteToLog("\r\n\r\n");
	
	SerialLog.WriteToLog("\r\nTest 4: Mutexes");
	Timer = GetMilli();
	ID = CreateProcess(&TestFour, "Test Four - Mutexes", 1);
	ProcessCheck = GetProcess(ID);
	ProcessCheck->Start();
	WaitForThread(ID, 0);
	Timer = GetMilli() - Timer;
	SerialLog.WriteToLog("\r\nTest 4 result: ");
	SerialLog.WriteToLog(Timer);
	SerialLog.WriteToLog("\r\n\r\n");
}

__attribute__((noinline)) void SystemIdle()
{
	while(1)
	{
		YieldCPU();
	}
}

