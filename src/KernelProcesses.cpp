/////////////////////////LIBRARY TOOLS////////////////////////////////////////////

__attribute__((noinline)) int CreateProcess(void* Main, const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void* Main, int ProcessID)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID));
	return ThreadID;
}

/*__attribute__((noinline)) int CreateQThread(void* Main, int ProcessID, int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID), "b"(Main), "c"(ProcessID), );
	return ThreadID;
}*/


/////////////////////////TEST PROGRAMS////////////////////////////////////////////

__attribute__((noinline)) volatile void Graphics()
{
	//Serial.WriteString(0x1, "\r\nDone");
	GUI.FrameAddress = (unsigned char*)((long)(CurrentThread->OwnerProcess)->MemStart);
	char shade = 255;
	char* MainFrame = (char*)GUI.FrameAddress;
	char* NewFrame = (char*)GUI.SecondFrameAddress;
	//Serial.WriteString(0x1, "\r\nWhile(1)");
	while(1)
	{
		/*for(int i = 0; i < GUI.Height; i++)
		for(int i = 0; i < GUI.Height; i++)
		{
			for(int j = 0; j < (GUI.Width*GUI.Depth); j++)
			{
				int pos = (i * GUI.BytesPerLine) + j;
				MainFrame[pos] = NewFrame[pos];
			}
		}*/
		for(long i = 0; i < (GUI.Height * GUI.BytesPerLine)/8; i++)
		((long*)MainFrame)[i] = ((long*)NewFrame)[i];
		YieldCPU();
	}
}

__attribute__((noinline)) volatile void SerialWrite()
{
	char NextLog[256];
	while(1)
	{
		SerialLog.ReadFromLog(NextLog);
		Serial.WriteString(0x1, "\r\n");
		Serial.WriteString(0x1, NextLog);
	}
}

__attribute__((noinline)) volatile void TaskManager()
{
	Draw Window = Draw(0, 0, GUI.Width/2, GUI.Height*3/4 + 7, GUI.Width, GUI.Height);
	SerialLog.WriteToLog("Shiet");
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
			int CurThreadID = 0;
			Window.DrawString(CurProcess->ProcessName, 19, y);
			y += 15;
			CurProcessID++;
			CurProcess = GetProcess(CurProcessID);
		}
		Window.Update();
		YieldCPU();
	}
}

/*int QuickThreadPeriod = 600;
int NormalThreadPeriod = 400;
bool CurrentThreadPeriod = 0; // 0 = Normal; 1 = Quick
int CurrentPeriodDuration = 0;
Thread* LastNormalThread;*/

__attribute__((noinline)) volatile void SystemIdle()
{
	SerialLog.WriteToLog("TEST SHIT");
	while(1)
	{
		YieldCPU();
		SerialLog.WriteToLog("Normal Thread Period");
		//SerialLog.WriteToLog(NormalThreadPeriod);
		SerialLog.WriteToLog("Current Period Duration");
		//SerialLog.WriteToLog(CurrentPeriodDuration);
	}
}
