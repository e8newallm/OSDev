void Scheduling()
{
	if(CurrentThreadDuration >= CurrentThread->MaxDuration)
	{
		//Serial.WriteString(0x1, "\r\n\r\nPreemptive swap");
		SwitchProcesses();
	}
}

__attribute__((noinline)) Process* GetProcess(int ID)
{
	return &(ProcessList[ID]);
}

Thread* Process::GetThread(int ID)
{
	return &(ThreadList[ID]);
}

__attribute__((noinline)) Thread* GetThread(int ProcessID, int ThreadID)
{
	Process* ProcessTemp = GetProcess(ProcessID);
	return &(ProcessTemp->ThreadList[ThreadID]);
}

int Process_Make(void* Main, const char* Name)
{
	for(long i = 0; i < 256; i++)
	{
		if(ProcessList[i].Available)
		{
			new (ProcessList+i) Process(Main, Name, (unsigned char)i);
			return i;
		}
	}
	Kernel_Panic("OH SHIT NO PROCESSES LEFT");
	return -1; // Stops pedantic warning
}

/////////////////////////PROCESS CODE///////////////////////////////////////

Process::Process(void* Main, const char* Name, unsigned char i)
{
	Available = false;
	ProcessID = i;
	ProcessName[255] = (char)0;
	for(int i = 0; i <= 254; i++)
	{
		ProcessName[i] = Name[i];
	}
	Page = PageFile();
	Page.SetupStartMemory();
	MemStart = (void*)(ProcessMemStart);
	unsigned long Temp = (unsigned long)PhysMemory.UseFreePhyAddr();
	Page.MapAddress(Temp, (unsigned long)(ProcessMemStart));
	MBlockHeader* BlockSetup = (MBlockHeader*)PhysicalAccess(Temp);
	StartBlock = (MBlockHeader*)ProcessMemStart;
	BlockSetup->PrevUsage = MBlockHeader_Start;
	BlockSetup->PrevSize = 0;
	BlockSetup->NextUsage = MBlockHeader_Free;
	BlockSetup->NextSize = 0x1000 - (sizeof(MBlockHeader)*2);
	BlockSetup = (MBlockHeader*)(((long)BlockSetup) + (0x1000 - sizeof(MBlockHeader)));
	BlockSetup->PrevUsage = MBlockHeader_Free;
	BlockSetup->PrevSize = 0x1000 - (sizeof(MBlockHeader)*2);
	BlockSetup->NextUsage = MBlockHeader_End;
	BlockSetup->NextSize = 0;
	EndBlock = (MBlockHeader*)(ProcessMemStart + 0x1000 - sizeof(MBlockHeader));
	Thread_Create(Main);
	Killed = false;
}


Process::Process()
{
	Available = true;
}

extern "C" void SwitchProcesses() //ROUND ROBIN
{
	CurrentThreadDuration = 0;
	Thread* Next = CurrentThread->NextThread;
	/*Thread* Test = Next;
	Serial.WriteString(0x1, "\r\n\r\nThreadLoop\r\nBefore(Thread: ");
	Serial.WriteString(0x1, Test->OwnerProcess->ProcessName);
	Serial.WriteString(0x1, " ");
	Serial.WriteLongHex(0x1, Test->ThreadID);
	Test = Next->NextThread;
	while(Test != Next)
	{
		Serial.WriteString(0x1, ")->(Thread: ");
		Serial.WriteString(0x1, Test->OwnerProcess->ProcessName);
		Serial.WriteString(0x1, " ");
		Serial.WriteLongHex(0x1, Test->ThreadID);
		Test = Test->NextThread;
	}
	Serial.WriteString(0x1, ")END");*/
	while(Next->State != THREADSTATE_RUNNING)
	{
		Next = Next->NextThread;
		CurrentThread->NextThread = Next;
	}
	/*Test = Next;
	Serial.WriteString(0x1, "\r\nAfter(Thread: ");
	Serial.WriteString(0x1, Test->OwnerProcess->ProcessName);
	Serial.WriteString(0x1, " ");
	Serial.WriteLongHex(0x1, Test->ThreadID);
	Test = Next->NextThread;
	while(Test != Next)
	{
		Serial.WriteString(0x1, ")->(Thread: ");
		Serial.WriteString(0x1, Test->OwnerProcess->ProcessName);
		Serial.WriteString(0x1, " ");
		Serial.WriteLongHex(0x1, Test->ThreadID);
		Test = Test->NextThread;
	}
	Serial.WriteString(0x1, ")END\r\n\r\n");*/
	long** NextRSP = &(Next->TSSRSP);
	long* NextPage = (Next->Page)->Pages;
	long** CurrentRSP = &(CurrentThread->TSSRSP);
	CurrentThread = Next;
	TSS.RSP0 = (unsigned long)CurrentThread->TSSRBP;
	SwitchASM(CurrentRSP, NextRSP, NextPage);
}

void StartProcesses()
{
	long* NextRSP = CurrentThread->TSSRSP;
	long* NextPage = (CurrentThread->Page)->Pages;
	TSS.RSP0 = (unsigned long)CurrentThread->TSSRBP;
	Output8(0x40, 0x9B);
	Output8(0x40, 0x2E);
	StartASM(NextRSP, NextPage);
}

void Process::Start()
{
	ThreadList[0].Start();
}

void Process::StartThread(int ThreadID)
{
	ThreadList[ThreadID].Start();
}

void Process::Kill()
{
	Killed = true;
	Available = true;
	Page.FreeAll();
	for(long i = 0; i < 256; i++)
	{
		if(ThreadList[i].State == THREADSTATE_RUNNING)
			ThreadList[i].Kill();
	}
	Thread* CurrTest = CurrentThread->NextThread;
	Thread* NextTest = CurrTest->NextThread;
	while(CurrTest != CurrentThread)
	{
		if(NextTest->OwnerProcess == this)
		{
			NextTest = NextTest->NextThread;
			CurrTest->NextThread = NextTest;
		}
	}
	//TODO: Add freeing of memory and closing of process if the current active one
}

/////////////////////////THREADING CODE///////////////////////////////////////

int Process::Thread_Create(void* Main)
{
	CLI();
	for(long i = 0; i < 256; i++)
	{
		if(ThreadList[i].State == THREADSTATE_AVAILABLE)
		{
			new (ThreadList+i) Thread(Main, this, &Page, i);
			return i;
		}
	}
	STI();
	return -1;
}

Thread::Thread(void* Main, Process* OwnerProcessIn, PageFile* PageIn, long IDThread)
{
	State = THREADSTATE_READY;
	ThreadID = IDThread;
	OwnerProcess = OwnerProcessIn;
	Page = PageIn;
	MaxDuration = 200;
	char* Stack = (char*)((StackSpaceStart + (IDThread * StackSize)));
	long Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)((Stack + TSSOffset - 1)));
	TSSRSP = (long*)((Stack + TSSOffset - 1) - (8 * 17)); //Add starting values to stack for ProcessSwitch to switch in
	TSSRBP = (long*)(Stack + TSSOffset - 1);
	long* StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 17));
	StackSetup[0] = 0x3200; //EFLAG starting value (IF=1 IOPL=2)
	StackSetup[16] = (long)&InitThread; //Code start address
	StackSetup[15] = (long)(Stack + StackSize - 1);
	Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)(Stack + StackSize - 0x1000));
	StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 2));
	StackSetup[0] = (long)Main; //End thread address
	StackSetup[1] = (long)&ReturnThread; //End thread address
	Page = &(OwnerProcess->Page);
}

void Thread::Start()
{
	CLI();
	State = THREADSTATE_RUNNING;
	NextThread = CurrentThread->NextThread;
	CurrentThread->NextThread = this;
	STI();
}

Thread::Thread()
{
	State = THREADSTATE_AVAILABLE;
}

void Thread::Kill()
{
	Thread* StartThreads = WaitingEndQueue;
	while(StartThreads != 0)
	{
		StartThreads->Start();
		StartThreads = StartThreads->WaitingEndQueueNext;
	}
	State = THREADSTATE_AVAILABLE;
	//TODO: Add freeing of memory and closing of process if the current active one
}

void Thread::Block()
{
	State = THREADSTATE_BLOCKED;
	//TODO: Add freeing of memory and closing of process if the current active one
}

extern "C" __attribute__((noinline))  void BeginThread()
{
	//SerialLog.WriteToLog("\r\nStarting Process: ");
	//SerialLog.WriteToLog(CurrentThread->OwnerProcess->ProcessName);
	return;
}

__attribute__((noinline)) void ReturnThread()
{
	/*Serial.WriteString(0x1, "\r\n(Ending Process: ");
	Serial.WriteString(0x1, CurrentThread->OwnerProcess->ProcessName);
	Serial.WriteString(0x1, " ");
	Serial.WriteLongHex(0x1, (long)CurrentThread->ThreadID);
	Serial.WriteString(0x1, ")\r\n");*/
	if(CurrentThread->ThreadID == 0)
		CurrentThread->OwnerProcess->Kill();
	else
		CurrentThread->Kill();
	YieldCPU();
}

void Mutex::Lock()
{
	while(!__sync_bool_compare_and_swap(&Editing, false, true))
	{
		YieldCPU();
	}
	if(!__sync_bool_compare_and_swap(&InUse, false, true))
	{
		Thread* Test = __sync_val_compare_and_swap(&QueueStart, 0, CurrentThread);
		if(Test == 0)
		{
			QueueEnd = CurrentThread;
			CurrentThread->NextThreadMutex = 0;
		}
		else
		{
			QueueEnd->NextThreadMutex = CurrentThread;
			QueueEnd = CurrentThread;
			CurrentThread->NextThreadMutex = 0;
		}
		CurrentThread->Block();
		Editing = false;
		YieldCPU();
	}
	else
	{
		Editing = false;
	}
	CurrentThreadMutex = CurrentThread;
}

void Mutex::Unlock()
{
	while(!__sync_bool_compare_and_swap(&Editing, false, true))
	{
		YieldCPU();
	}
	if(CurrentThreadMutex == CurrentThread)
	{
		if(QueueStart != 0)
		{;
			CurrentThreadMutex = QueueStart;
			QueueStart = QueueStart->NextThreadMutex;
			CurrentThreadMutex->Start();
		}
		else
		{
			CurrentThreadMutex = 0;
			InUse = false;
		}
	}
	Editing = false;
}

void YieldCPU()
{
	asm("int $0x50");
}

template <typename ListType, unsigned int S> ListType FIFOList<ListType, S>::Read()
{
	if(Head == Tail)
	
	ListType ReturnValue = List[Tail];
	Tail++;
	if(Tail > S)
	{
		Tail = 0;
		SameCycle = true;
	}
}

template <typename ListType, unsigned int S> void FIFOList<ListType, S>::Write(ListType Message)
{
	List[Head] = Message;
	Head++;
	if(Head > S)
	{
		Head = 0;
		SameCycle = false;
	}
}

template <typename ListType, unsigned int S> bool FIFOList<ListType, S>::Writable()
{
	if(Head != Tail && SameCycle)
		return false;
	return true;
}

template <typename ListType, unsigned int S> bool FIFOList<ListType, S>::Readable()
{
	if(Head != Tail && !SameCycle)
		return false;
	return true;
}
