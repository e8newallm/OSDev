/////////////////////////LIBRARY TOOLS////////////////////////////////////////////


__attribute__((noinline)) void StartThread(int ThreadID)
{
	asm volatile("INT $0x51" : : "a"(THREAD_START_ID), "b"(ThreadID));
}

__attribute__((noinline)) int CreateProcess(void Main(), const char* Name, int Priority)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name), "d"(Priority));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void Main(), int ProcessID, int Priority)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID), "d"(Priority));
	return ThreadID;
}

__attribute__((noinline)) int CreateThread(void Main(), int Priority)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main), "c"(Priority));
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

__attribute__((noinline)) long GetMilli()
{
	long Milli = 0;
	asm volatile("INT $0x51" : "=a"(Milli) : "a"(GET_MILLI_ID));
	return Milli;
}

__attribute__((noinline)) void WaitForThread(int Process, int Thread)
{
	asm volatile("INT $0x51" : : "a"(WAIT_THREAD_ID), "b"(Process), "c"(Thread));
	YieldCPU();
}
