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
	//Serial.WriteString(0x1, "\r\nNEW SWATPEGGGEWFGEWF");
	while(Next == 0)
	{
		//Serial.WriteString(0x1, "\r\nLoop");
		if(UIThreadDuration < UIThreadPeriod && UIThread != 0)
		{
			//Serial.WriteString(0x1, "\r\nTrying UI");
			Thread* PotentialNext = UIThread->NextThread;
			while(PotentialNext->State != THREADSTATE_RUNNING)
			{
				//Serial.WriteString(0x1, "\r\n\tLOOP");
				if(PotentialNext == UIThread)
				{
					UIThread = 0;
					PotentialNext = 0;
					break;
				}
				UIThread->NextThread = PotentialNext->NextThread;
				PotentialNext = UIThread->NextThread;
			}
			if(PotentialNext != 0)
			{
				CurrentThreadPeriod = &UIThreadPeriod;
				CurrentPeriodDuration = &UIThreadDuration;
				UIThread = PotentialNext;
				Next = PotentialNext;
				break;
			}
		}
		if(NormalThreadDuration < NormalThreadPeriod && NormalThread != 0)
		{
			//Serial.WriteString(0x1, "\r\nTrying Normal");
			Thread* PotentialNext = NormalThread->NextThread;
			//Serial.WriteString(0x1, "\r\nPotentialNext: ");
			//Serial.WriteString(0x1, PotentialNext->OwnerProcess->ProcessName);
			while(PotentialNext->State != THREADSTATE_RUNNING)
			{
				//Serial.WriteString(0x1, "\r\n\tLOOP");
				if(PotentialNext == NormalThread)
				{
					NormalThread = 0;
					PotentialNext = 0;
					break;
				}
				NormalThread->NextThread = PotentialNext->NextThread;
				PotentialNext = NormalThread->NextThread;
				//Serial.WriteString(0x1, "\r\nPotentialNext: ");
				//Serial.WriteString(0x1, PotentialNext->OwnerProcess->ProcessName);
			}
			if(PotentialNext != 0)
			{
				CurrentThreadPeriod = &NormalThreadPeriod;
				CurrentPeriodDuration = &NormalThreadDuration;
				NormalThread = PotentialNext;
				Next = PotentialNext;
				break;
			}
		}
		if(BackgroundThreadDuration < BackgroundThreadPeriod && BackgroundThread != 0)
		{
			//Serial.WriteString(0x1, "\r\nTrying Back");
			Thread* PotentialNext = BackgroundThread->NextThread;
			while(PotentialNext->State != THREADSTATE_RUNNING)
			{
				//Serial.WriteString(0x1, "\r\n\tLOOP");
				if(PotentialNext == BackgroundThread)
				{
					BackgroundThread = 0;
					PotentialNext = 0;
					break;
				}
				BackgroundThread->NextThread = PotentialNext->NextThread;
				PotentialNext = BackgroundThread->NextThread;
			}
			if(PotentialNext != 0)
			{
				CurrentThreadPeriod = &BackgroundThreadPeriod;
				CurrentPeriodDuration = &BackgroundThreadDuration;
				BackgroundThread = PotentialNext;
				Next = PotentialNext;
				break;
			}
		}
		if(BackgroundThreadDuration >= BackgroundThreadPeriod || NormalThreadDuration >= NormalThreadPeriod || UIThreadDuration >= UIThreadDuration)
		{
			//Serial.WriteString(0x1, "\r\nFailed! Resetting periods");
			BackgroundThreadDuration = 0;
			NormalThreadDuration = 0;
			UIThreadDuration = 0;
		}
		else
			Next = GetProcess(0)->GetThread(0); //IDLE
	}
	//Serial.WriteString(0x1, "\r\nStarting thread from ");
	//Serial.WriteString(0x1, Next->OwnerProcess->ProcessName);
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
	Thread* CurrTest = UIThread->NextThread;
	Thread* NextTest;
	if(UIThread != 0)
	{
		NextTest = CurrTest->NextThread;
		while(CurrTest != UIThread)
		{
			if(NextTest->OwnerProcess == this)
			{
				NextTest = NextTest->NextThread;
				CurrTest->NextThread = NextTest;
			}
		}
	}
	
	CurrTest = NormalThread->NextThread;
	if(NormalThread != 0)
	{
		NextTest = CurrTest->NextThread;
		while(CurrTest != NormalThread)
		{
			if(NextTest->OwnerProcess == this)
			{
				NextTest = NextTest->NextThread;
				CurrTest->NextThread = NextTest;
			}
		}
	}
	
	CurrTest = BackgroundThread->NextThread;
	if(BackgroundThread != 0)
	{
		NextTest = CurrTest->NextThread;
		while(CurrTest != BackgroundThread)
		{
			if(NextTest->OwnerProcess == this)
			{
				NextTest = NextTest->NextThread;
				CurrTest->NextThread = NextTest;
			}
		}
	}
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
	MaxDuration = 20000;
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
	switch(Type)
	{
		case THREADTYPE_UI:
		{
			if(UIThread == 0)
				UIThread = this;
			NextThread = UIThread->NextThread;
			UIThread->NextThread = this;
			break;
		}
		case THREADTYPE_NORM:
		{
			if(NormalThread == 0)
				NormalThread = this;
			NextThread = NormalThread->NextThread;
			NormalThread->NextThread = this;
			break;
		}
		case THREADTYPE_BACK:
		{
			if(BackgroundThread == 0)
				BackgroundThread = this;
			NextThread = BackgroundThread->NextThread;
			BackgroundThread->NextThread = this;
			break;
		}
	}
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
	return;
}

__attribute__((noinline)) void ReturnThread()
{
	if(CurrentThread->ThreadID == 0)
	{
		CurrentThread->OwnerProcess->Kill();
	}
	Thread* StartThreads = CurrentThread->WaitingEndQueue;
	while(StartThreads != 0)
	{
		StartThreads->Start();
		StartThreads = StartThreads->WaitingEndQueueNext;
	}
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
