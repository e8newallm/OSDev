/////////////////////////LIBRARY TOOLS////////////////////////////////////////////


__attribute__((noinline)) void StartThread(int ThreadID)
{
	asm volatile("INT $0x51" : : "a"(THREAD_START_ID), "b"(ThreadID));
}

#ifdef ProcessQ
__attribute__((noinline)) int CreateProcess(void Main(), const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void Main(), int ProcessID)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID));
	return ThreadID;
}

__attribute__((noinline)) int CreateQThread(void Main(), int ProcessID, int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID), "b"(Main), "c"(ProcessID), "d"(Duration));
	return ThreadID;
}

__attribute__((noinline)) int CreateThread(void Main())
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main));
	return ThreadID;
}

__attribute__((noinline)) int CreateQThread(void Main(), int Duration)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(QTHREAD_MAKE_ID_SELF), "b"(Main), "d"(Duration));
	return ThreadID;
}
#endif

#ifdef ProcessRR
__attribute__((noinline)) int CreateProcess(void Main(), const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void Main(), int ProcessID)
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID), "b"(Main), "c"(ProcessID));
	return ThreadID;
}

__attribute__((noinline)) int CreateThread(void Main())
{

	long ThreadID = 0;
	asm volatile("INT $0x51" : "=a"(ThreadID) : "a"(THREAD_MAKE_ID_SELF), "b"(Main));
	return ThreadID;
}

#endif

#ifdef ProcessPBS
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
#endif

#ifdef ProcessMQS

__attribute__((noinline)) int CreateProcess(void Main(), const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateBackProcess(void Main(), const char* Name)
{

	long ProcessID = 0;
	asm volatile("INT $0x51" : "=a"(ProcessID) : "a"(BACK_PROCESS_MAKE_ID), "b"(Main), "c"(Name));
	return ProcessID;
}

__attribute__((noinline)) int CreateThread(void Main(), int Priority)
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

__attribute__((noinline)) void WaitForThread(int Process, int Thread)
{
	asm volatile("INT $0x51" : : "a"(WAIT_THREAD_ID), "b"(Process), "c"(Thread));
	YieldCPU();
}
