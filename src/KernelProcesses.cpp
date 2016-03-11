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

__attribute__((noinline)) int CreateQThread(void* Main, int ProcessID, int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID), "b"(Main), "c"(ProcessID), "d"(Duration));
	return ThreadID;
}

__attribute__((noinline)) void MapVideoMemory(void* Start)
{
	asm volatile("INT $0x51" : : "a"(MAP_VIDEO_MEM_ID), "b"(Start));
}

__attribute__((noinline)) void UpdateWindow(Draw* Data)
{
	asm volatile("INT $0x51" : : "a"(UPDATE_WINDOW_ID), "b"(Data));
}


/*__attribute__((noinline)) void YieldCPU()
{
	asm("int $0x50");
}*/

/////////////////////////TEST PROGRAMS////////////////////////////////////////////
///////TODO: COMPILE SEPARATELY AND LOAD USING GRUB MODULES///////////////////////

__attribute__((noinline)) volatile void Graphics()
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

__attribute__((noinline)) volatile void SystemIdle()
{
	while(1)
	{
		YieldCPU();
	}
}
