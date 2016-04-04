/////////////////////////LIBRARY TOOLS////////////////////////////////////////////

__attribute__((noinline)) int CreateProcess(void* Main, const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

#ifdef ProcessQ
__attribute__((noinline)) int CreateThread(void* Main, int ProcessID)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID));
	return ThreadID;
}

__attribute__((noinline)) int CreateQThread(void* Main, int ProcessID, int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID), "b"(Main), "c"(ProcessID), "d"(Duration));
	return ThreadID;
}

__attribute__((noinline)) int CreateThread(void* Main)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main));
	return ThreadID;
}

__attribute__((noinline)) int CreateQThread(void* Main, int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID_SELF), "b"(Main), "d"(Duration));
	return ThreadID;
}
#endif

#ifdef ProcessPBS
__attribute__((noinline)) int CreateThread(void* Main, int ProcessID, int Priority)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID), "d"(Priority));
	return ThreadID;
}

__attribute__((noinline)) int CreateThread(void* Main, int Priority)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main), "c"(Priority));
	return ThreadID;
}
#endif

#ifdef ProcessMQS

__attribute__((noinline)) int CreateBackProcess(void* Main, const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(BACK_PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void* Main, int Priority)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main), "c"(Priority));
	return ThreadID;
}
#endif

__attribute__((noinline)) void MapVideoMemory(void* Start)
{
	asm volatile("INT $0x51" : : "a"(MAP_VIDEO_MEM_ID), "b"(Start));
}

__attribute__((noinline)) void UpdateWindow(Draw* Data)
{
	asm volatile("INT $0x51" : : "a"(UPDATE_WINDOW_ID), "b"(Data));
}

__attribute__((noinline)) long GetMilli()
{
	long Milli = 0;
	asm volatile("INT $0x51" : "=a"(Milli) : "a"(GET_MILLI_ID));
	return Milli;
}

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
		Serial.WriteString(0x1, "\r\n");
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

volatile long Result = 0;
volatile long Result2 = 0;

__attribute__((noinline)) void TestShit()
{
	long Temp = 0;
	SerialLog.WriteToLog("Test shit is running yo");
	for(int i = 0; i < 200000; i++)
	{
		Temp += i;
	}
	Result = Temp;
	return;
}

__attribute__((noinline)) void TestShitTwo()
{
	long Temp = 0;
	SerialLog.WriteToLog("Test shit is running yo");
	for(int i = 0; i < 200000000; i++)
	{
		Temp += i;
	}
	Result2 = Temp;
	return;
}

__attribute__((noinline)) void SystemIdle()
{
	YieldCPU();
	SerialLog.WriteToLog("\r\nStarting Test. Millis: ");
	long Millis = GetMilli();
	SerialLog.WriteToLog(Millis);
	while(Result == 0)
	{
		SerialLog.WriteToLog("\r\nResult Pending");
		YieldCPU();
	}
	SerialLog.WriteToLog("\r\nThe result is ");
	SerialLog.WriteToLog(Result);
	while(Result2 == 0)
	{
		SerialLog.WriteToLog("\r\nResult2 Pending");
		YieldCPU();
	}
	SerialLog.WriteToLog("\r\nThe result2 is ");
	SerialLog.WriteToLog(Result2);
	SerialLog.WriteToLog("Final Millis: ");
	long FinalMillis = GetMilli();
	SerialLog.WriteToLog(FinalMillis);

	SerialLog.WriteToLog("Total Duration: ");
	SerialLog.WriteToLog(FinalMillis - Millis);
	
	while(1)
	{
		YieldCPU();
	}
}
