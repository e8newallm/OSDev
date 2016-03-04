extern "C" void SwitchProcesses();
extern tss_entry TSS;


void* Malloc(unsigned long);
void ReturnThread();
void BeginThread();

struct MBlockHeader 
{
	unsigned char PrevUsage;
	long PrevSize;
	long NextSize;
	unsigned char NextUsage;
} __attribute__((packed));



class Process;

class Thread
{
	public:
	char Type; //0 = Normal; 1 = Quick
	long Test = 0xDEADBEEF;
	bool Available;
	long* TSSRSP;
	long* TSSRBP;
	int ThreadID;
	long Duration, MaxDuration;
	PageFile* Page;
	Process* OwnerProcess;
	Thread* NextThread;
	bool Killed;
	Thread(void*, Process*, PageFile*, long);
	Thread(void*, Process*, PageFile*, long, int);
	Thread();
	void Start();
	void Kill();
};

class Process
{
public:
	long Test = 0xDEADBEEF;
	bool Available;
	PageFile Page;
	void* MemStart;
	MBlockHeader* StartBlock;
	MBlockHeader* EndBlock;
	bool Killed;
	Process();
	Process(void*, const char*);
	int Thread_Create(void*);
	int QThread_Create(void*, int);
	void Start();
	void Kill();
	void StartThread(int);
	Thread ThreadList[256];
	char ProcessName[256];
	Thread* GetThread(int);
};

Thread* CurrentThread;
int CurrentThreadDuration;

//Shortest remaining time
volatile long QuickThreadPeriod = 600;
volatile long NormalThreadPeriod = 400;
volatile bool CurrentThreadPeriod = 0; // 0 = Normal; 1 = Quick
volatile long CurrentPeriodDuration = 123;
Thread* LastNormalThread;

Process ProcessList[256];

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
			new (ProcessList+i) Process(Main, Name);
			return i;
		}
	}
	Kernel_Panic("OH SHIT NO PROCESSES LEFT");
}

/////////////////////////PROCESS CODE///////////////////////////////////////

Process::Process(void* Main, const char* Name)
{
	Available = false;
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

extern "C" void SwitchProcesses() //SHORTEST REMAINING TIME
{
	asm("PUSH %RBP; PUSH %RAX; PUSH %RCX; PUSH %RBX; PUSH %RDX; PUSH %RSI; PUSH %RDI; PUSH %R8; PUSH %R9; PUSH %R10; PUSH %R11; PUSH %R12; PUSH %R13; PUSH %R14; PUSH %R15; PUSHF");
	CurrentThreadDuration = 0;
	
	Thread* Next = (Thread*)0;
	if(CurrentThreadPeriod)
	{
		SerialLog.WriteToLog("Quickthread time");
		for(unsigned char i = 0; i < 256; i++)
		{
			Process* CurProcess = GetProcess(i);
			if(CurProcess->Available)
			{
				for(unsigned char j = 0; j < 256; j++)
				{
					Thread* CurThread = CurProcess->GetThread(j);
					if(CurThread->Type && (Next == (Thread*)0 || CurThread->MaxDuration < Next->MaxDuration))
						Next = CurThread;
				}
			}
		}
		if(Next == (Thread*)0) //No quickthreads
		{
			SerialLog.WriteToLog("\tNo quickthreads");
			CurrentThread = LastNormalThread;
			CurrentThreadPeriod = !CurrentThreadPeriod;
		}
	}
	if(!CurrentThreadPeriod)
	{
		//SerialLog.WriteToLog("\r\nNormal thread time");
		Next = CurrentThread->NextThread;
		while(Next->Killed)
		{
			Next->Available = true;
			Next = Next->NextThread;
			CurrentThread->NextThread = Next;
		}
	}
	
	asm volatile("MOV %%RSP, %0" : "=r"(CurrentThread->TSSRSP));
	long* NextRSP = Next->TSSRSP;
	long* NextPage = (Next->Page)->Pages;
	CurrentThread = CurrentThread->NextThread;
	asm volatile("MOV %1, %%CR3; MOV %0, %%RSP;"
			 : :  "r"(NextRSP), "r"(NextPage));
	TSS.RSP0 = (unsigned long)CurrentThread->TSSRBP;
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RCX; POP %RAX; POP %RBP");
}

/*extern "C" void SwitchProcesses() //ROUND ROBIN
{
	asm("PUSH %RBP; PUSH %RAX; PUSH %RCX; PUSH %RBX; PUSH %RDX; PUSH %RSI; PUSH %RDI; PUSH %R8; PUSH %R9; PUSH %R10; PUSH %R11; PUSH %R12; PUSH %R13; PUSH %R14; PUSH %R15; PUSHF");
	CurrentThreadDuration = 0;
	Thread* Next = CurrentThread->NextThread;
	while(Next->Killed)
	{
		Next->Available = true;
		Next = Next->NextThread;
		CurrentThread->NextThread = Next;
	}
	asm volatile("MOV %%RSP, %0" : "=r"(CurrentThread->TSSRSP));
	long* NextRSP = Next->TSSRSP;
	long* NextPage = (Next->Page)->Pages;
	CurrentThread = CurrentThread->NextThread;
	asm volatile("MOV %1, %%CR3; MOV %0, %%RSP;"
			 : :  "r"(NextRSP), "r"(NextPage));
	TSS.RSP0 = (unsigned long)CurrentThread->TSSRBP;
	//short NewDuration = 1193182 * (CurrentProcess->Duration / 1000);
	//Output8(0x40, NewDuration&0xFF); //Set lower byte 0x4E
	//Output8(0x40, NewDuration>>8); //set higher byte 0x17
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RCX; POP %RAX; POP %RBP");
}*/

void StartProcesses()
{
	long* NextRSP = CurrentThread->TSSRSP;
	long* NextPage = (CurrentThread->Page)->Pages;
	asm volatile("MOV %1, %%CR3; MOV %0, %%RSP;"
				 : : "r"(NextRSP), "r"(NextPage));
	TSS.RSP0 = (unsigned long)CurrentThread->TSSRBP;
	short NewDuration = 0x2E9B;
	Output8(0x40, NewDuration&0xFF); //Set lower byte 0x4E
	Output8(0x40, NewDuration>>8); //set higher byte 0x17
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RCX; POP %RAX; POP %RBP");
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

int Process::Thread_Create(void* Main)
{
	for(long i = 0; i < 256; i++)
	{
		if(ThreadList[i].Available)
		{
			new (ThreadList+i) Thread(Main, this, &Page, i);
			return i;
		}
	}
	return -1;
}

int Process::QThread_Create(void* Main, int Duration)
{
	for(long i = 0; i < 256; i++)
	{
		if(ThreadList[i].Available)
		{
			new (ThreadList+i) Thread(Main, this, &Page, i, Duration);
			return i;
		}
	}
	return -1;
}

Thread::Thread(void* Main, Process* OwnerProcessIn, PageFile* PageIn, long IDThread)
{
	Type = 0;
	Available = false;
	ThreadID = IDThread;
	OwnerProcess = OwnerProcessIn;
	Page = PageIn;
	MaxDuration = 200;
	char* Stack = (char*)((StackSpaceStart + (IDThread * StackSize)));
	long Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)((Stack + TSSOffset - 1)));
	TSSRSP = (long*)((Stack + TSSOffset - 1) - (8 * 17)); //Add starting values to stack for ProcessSwitch to switch in
	TSSRBP = (long*)(Stack + TSSOffset - 8);
	long* StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 17));
	StackSetup[0] = 0x3200; //EFLAG starting value (IF=1 IOPL=2)
	StackSetup[16] = (long)&BeginThread; //Code start address
	StackSetup[15] = (long)(Stack + StackSize - 1);
	Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)(Stack + StackSize - 0x1000));
	StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 2));
	StackSetup[0] = (long)Main; //End thread address
	StackSetup[1] = (long)&ReturnThread; //End thread address
	Page = &(OwnerProcess->Page);
	Killed = false;
}

Thread::Thread(void* Main, Process* OwnerProcessIn, PageFile* PageIn, long IDThread, int DurationIn)
{
	Type = 1;
	Available = false;
	ThreadID = IDThread;
	OwnerProcess = OwnerProcessIn;
	Page = PageIn;
	MaxDuration = DurationIn;
	char* Stack = (char*)((StackSpaceStart + (IDThread * StackSize)));
	long Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)((Stack + TSSOffset - 1)));
	TSSRSP = (long*)((Stack + TSSOffset - 1) - (8 * 17)); //Add starting values to stack for ProcessSwitch to switch in
	TSSRBP = (long*)(Stack + TSSOffset - 8);
	long* StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 17));
	StackSetup[0] = 0x3200; //EFLAG starting value (IF=1 IOPL=2)
	StackSetup[16] = (long)&BeginThread; //Code start address
	StackSetup[15] = (long)(Stack + StackSize - 1);
	Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)(Stack + StackSize - 0x1000));
	StackSetup = PhysicalAccess(Temp  + 0xFFF - (8 * 2));
	StackSetup[0] = (long)Main; //End thread address
	StackSetup[1] = (long)&ReturnThread; //End thread address
	Page = &(OwnerProcess->Page);
	Killed = false;
}


void Thread::Start()
{
	NextThread = CurrentThread->NextThread;
	CurrentThread->NextThread = this;
}

Thread::Thread()
{
	Available = true;
}

void Thread::Kill()
{
	Killed = true;
	//TODO: Add freeing of memory and closing of process if the current active one
}

__attribute__((noinline)) void BeginThread()
{
	asm volatile("MOV %RBP, %RSP; SUB $0x10, %RSP; MOV %RSP, %RAX; PUSH $0x23; PUSH %RAX; PUSHF; PUSH $0x1B; PUSH $1f; IRETQ; 1:");
	//Serial.WriteString(0x1, "\r\nStarting Process: ");
	//Serial.WriteString(0x1, CurrentThread->OwnerProcess->ProcessName);
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
	while(!__sync_bool_compare_and_swap(&InUse, false, true))
	{
		YieldCPU();
	}
	CurrentThreadID = CurrentThread->ThreadID;
}

void Mutex::Unlock()
{
	if(CurrentThreadID == CurrentThread->ThreadID)
	{
		CurrentThreadID = 0;
		InUse = false;
	}
}

void YieldCPU()
{
	asm("int $0x50");
}

void CriticalRegion::Lock()
{
	RegionMutex.Lock();
}

void CriticalRegion::Unlock()
{
	RegionMutex.Unlock();
}
