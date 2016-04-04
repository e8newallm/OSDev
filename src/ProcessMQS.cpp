void Scheduling()
{
	if(*CurrentPeriodDuration >= *CurrentThreadPeriod || CurrentThreadDuration >= CurrentThread->Duration)
	{
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
			new (ProcessList+i) Process(Main, Name, (unsigned char)i, false);
			return i;
		}
	}
	Kernel_Panic("OH SHIT NO PROCESSES LEFT");
	return -1; // Stops pedantic warning
}

int Back_Process_Make(void* Main, const char* Name)
{
	for(long i = 0; i < 256; i++)
	{
		if(ProcessList[i].Available)
		{
			new (ProcessList+i) Process(Main, Name, (unsigned char)i, true);
			return i;
		}
	}
	Kernel_Panic("OH SHIT NO PROCESSES LEFT");
	return -1; // Stops pedantic warning
}

/////////////////////////PROCESS CODE///////////////////////////////////////

Process::Process(void* Main, const char* Name, unsigned char i, bool Background)
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
	if(Background)
		Thread_Create(Main, THREADTYPE_BACK);
	else
		Thread_Create(Main, THREADTYPE_NORM);
	Killed = false;
}


Process::Process()
{
	Available = true;
}

extern "C" void SwitchProcesses() //MULTI QUEUE SCHEDULER
{
	CurrentThreadDuration = 0;
	CurrentThread->LastUsage = TimeSinceStart;
	Thread* Next = (Thread*)0;
	
	while(Next == 0)
	{ 
		if(UIThreadDuration < UIThreadPeriod)
		{
			for(unsigned short i = 0; i < 256; i++)
			{
				Process* CurProcess = GetProcess(i);
				if(!CurProcess->Available)
				{
					for(unsigned short j = 0; j < 256; j++)
					{
						Thread* CurThread = CurProcess->GetThread(j);
						if(CurThread->State == THREADSTATE_RUNNING && CurThread->Type == THREADTYPE_UI)
						{
							if(Next == (Thread*)0 || CurThread->LastUsage < Next->LastUsage)
								Next = CurThread;
						}
					}
				}
			}
			if(Next != 0)
			{
				CurrentThreadPeriod = &UIThreadPeriod;
				CurrentPeriodDuration = &UIThreadDuration;
				break;
			}
		}
		if(NormalThreadDuration < NormalThreadPeriod)
		{
			for(unsigned short i = 0; i < 256; i++)
			{
				Process* CurProcess = GetProcess(i);
				if(!CurProcess->Available)
				{
					for(unsigned short j = 0; j < 256; j++)
					{
						Thread* CurThread = CurProcess->GetThread(j);
						if(CurThread->State == THREADSTATE_RUNNING && CurThread->Type == THREADTYPE_NORM)
						{
							if(Next == (Thread*)0 || CurThread->LastUsage < Next->LastUsage)
								Next = CurThread;
						}
					}
				}
			}
			if(Next != 0)
			{
				CurrentThreadPeriod = &NormalThreadPeriod;
				CurrentPeriodDuration = &NormalThreadDuration;
				break;
			}
		}
		if(BackgroundThreadDuration < BackgroundThreadPeriod)
		{
			for(unsigned short i = 0; i < 256; i++)
			{
				Process* CurProcess = GetProcess(i);
				if(!CurProcess->Available)
				{
					for(unsigned short j = 0; j < 256; j++)
					{
						Thread* CurThread = CurProcess->GetThread(j);
						if(CurThread->State == THREADSTATE_RUNNING && CurThread->Type == THREADTYPE_BACK)
						{
							if(Next == (Thread*)0 || CurThread->LastUsage < Next->LastUsage)
								Next = CurThread;
						}
					}
				}
			}
			if(Next != 0)
			{
				CurrentThreadPeriod = &BackgroundThreadPeriod;
				CurrentPeriodDuration = &BackgroundThreadDuration;
				break;
			}
		}
		if(BackgroundThreadDuration >= BackgroundThreadPeriod || NormalThreadDuration >= NormalThreadPeriod || UIThreadDuration >= UIThreadDuration)
		{
			BackgroundThreadDuration = 0;
			NormalThreadDuration = 0;
			UIThreadDuration = 0;
		}
		else
			Next = GetProcess(0)->GetThread(0); //IDLE
	}
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
	//TODO: Add freeing of memory and closing of process if the current active one
}

/////////////////////////THREADING CODE///////////////////////////////////////

int Process::Thread_Create(void* Main, int Priority)
{
	CLI();
	for(long i = 0; i < 256; i++)
	{
		if(ThreadList[i].State == THREADSTATE_AVAILABLE)
		{
			new (ThreadList+i) Thread(Main, this, &Page, i, Priority);
			return i;
		}
	}
	STI();
	return -1;
}


Thread::Thread(void* Main, Process* OwnerProcessIn, PageFile* PageIn, long IDThread, int Priority)
{
	Type = Priority;
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
	State = THREADSTATE_RUNNING;
}

Thread::Thread()
{
	State = THREADSTATE_AVAILABLE;
}

void Thread::Kill()
{
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
	SerialLog.WriteToLog("\r\nStarting Process: ");
	SerialLog.WriteToLog(CurrentThread->OwnerProcess->ProcessName);
	return;
}

__attribute__((noinline)) void ReturnThread()
{
	CurrentThread->Kill();
	if(CurrentThread->ThreadID == 0)
	{
		CurrentThread->OwnerProcess->Kill();
	}
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
		{
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
