extern "C" void SwitchProcesses();
extern tss_entry TSS;
#define MBlockHeader_Free 0
#define MBlockHeader_InUse 1
#define MBlockHeader_Start (unsigned char)254
#define MBlockHeader_End (unsigned char)255


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
	long Test = 0xDEADBEEF;
	bool Available;
	long* TSSRSP;
	long* TSSRBP;
	int ThreadID;
	long Duration, MaxDuration;
	PageFile* Page;
	Process* OwnerProcess;
	Thread* NextProcess;
	bool Killed;
	Thread(void*, Process*, PageFile*, long);
	Thread();
	void Start();
	void Kill();
};

class Process
{
public:
	long Test = 0xDEADBEEF;
	bool Available;
	long Duration, MaxDuration;
	PageFile Page;
	void* MemStart;
	MBlockHeader* StartBlock;
	MBlockHeader* EndBlock;
	bool Killed;
	Process();
	Process(void*, const char*);
	int CreateThread(void*);
	void Start();
	void Kill();
	void StartThread(int);
	Thread ThreadList[256];
	char ProcessName[256];
	Thread* GetThread(int);
};

Thread* CurrentProcess;
Process ProcessList[256];

Process* GetProcess(int ID)
{
	return &(ProcessList[ID]);
}

Thread* Process::GetThread(int ID)
{
	return &(ThreadList[ID]);
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

#define StackSize 0x100000
#define TSSOffset 0x80000
#define StackSpaceStart 0xFF0000000
#define StackSpaceEnd StackSpaceStart + (StackSize * 255)
Process::Process(void* Main, const char* Name)
{
	Available = false;
	ProcessName[255] = (char)0;
	for(int i = 0; i <= 254; i++)
	{
		ProcessName[i] = Name[i];
	}
	Duration = 0;
	MaxDuration = 30;
	Page = PageFile();
	Page.SetupStartMemory();
	MemStart = (void*)(ProcessMemStart);
	Page.MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(ProcessMemStart));
	MBlockHeader* BlockSetup = (MBlockHeader*)PhysicalAccess(ProcessMemStart);
	StartBlock = BlockSetup;
	BlockSetup->PrevUsage = MBlockHeader_Start;
	BlockSetup->PrevSize = 0;
	BlockSetup->NextUsage = MBlockHeader_Free;
	BlockSetup->NextSize = 0x1000 - (sizeof(MBlockHeader)*2);
	BlockSetup = (MBlockHeader*)(((long)BlockSetup) + (0x1000 - sizeof(MBlockHeader)));
	BlockSetup->PrevUsage = MBlockHeader_Free;
	BlockSetup->PrevSize = 0x1000 - (sizeof(MBlockHeader)*2);
	BlockSetup->NextUsage = MBlockHeader_End;
	BlockSetup->NextSize = 0;
	EndBlock = BlockSetup;
	CreateThread(Main);
	Killed = false;
}

Process::Process()
{
	Available = true;
}

extern "C" void SwitchProcesses()
{
	if(Multitasking)
	{
		asm("PUSH %RBP; PUSH %RAX; PUSH %RCX; PUSH %RBX; PUSH %RDX; PUSH %RSI; PUSH %RDI; PUSH %R8; PUSH %R9; PUSH %R10; PUSH %R11; PUSH %R12; PUSH %R13; PUSH %R14; PUSH %R15; PUSHF");
		CurrentProcess->Duration = 0;
		Thread* Next = CurrentProcess->NextProcess;
		while(Next->Killed)
		{
			Next->Available = true;
			Next = Next->NextProcess;
			CurrentProcess->NextProcess = Next;
		}
		asm volatile("MOV %%RSP, %0" : "=r"(CurrentProcess->TSSRSP));
		long* NextRSP = Next->TSSRSP;
		long* NextPage = (Next->Page)->Pages;
		CurrentProcess = CurrentProcess->NextProcess;
		CurrentProcess->Duration = 0;
		asm volatile("MOV %1, %%CR3; MOV %0, %%RSP;"
				 : :  "r"(NextRSP), "r"(NextPage));
		TSS.RSP0 = (unsigned long)CurrentProcess->TSSRBP;
		asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RCX; POP %RAX; POP %RBP");
	}
	Output8(0x40, 0x4E); //Set lower byte 0x4E
	Output8(0x40, 0x17); //set higher byte 0x17
}

void StartProcesses()
{
	CurrentProcess->Duration = 0;
	long* NextRSP = CurrentProcess->TSSRSP;
	long* NextPage = (CurrentProcess->Page)->Pages;
	Output8(0x40, 0x4E); //Set lower byte 0x4E
	Output8(0x40, 0x17); //set higher byte 0x17
	Multitasking = true;
	asm volatile("MOV %1, %%CR3; MOV %0, %%RSP;"
				 : : "r"(NextRSP), "r"(NextPage));
	TSS.RSP0 = (unsigned long)CurrentProcess->TSSRBP;
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

int Process::CreateThread(void* Main)
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

Thread::Thread(void* Main, Process* OwnerProcessIn, PageFile* PageIn, long IDThread)
{
	Available = false;
	ThreadID = IDThread;
	OwnerProcess = OwnerProcessIn;
	Page = PageIn;
	char* Stack = (char*)((StackSpaceStart + (IDThread * StackSize)));
	long Temp = (long)PhysMemory.UseFreePhyAddr();
	Page->MapAddress(Temp, (unsigned long)((Stack + TSSOffset - 1)));
	TSSRSP = (long*)((Stack + TSSOffset - 1) - (8 * 17)); //Add starting values to stack for ProcessSwitch to switch in
	TSSRBP = (long*)(Stack + TSSOffset - 8);
	Serial.WriteString(0x1, "\r\nRSP: ");
	Serial.WriteLongHex(0x1, TSSRSP);
	Serial.WriteString(0x1, "\r\nRBP: ");
	Serial.WriteLongHex(0x1, TSSRBP);
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
	NextProcess = CurrentProcess->NextProcess;
	CurrentProcess->NextProcess = this;
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
	asm("MOV %RBP, %RSP; SUB $0x10, %RSP; MOV %RSP, %RAX; PUSH $0x23; PUSH %RAX; PUSHF; PUSH $0x1B; PUSH $1f; IRETQ; 1:");
	return;
}

__attribute__((noinline)) void ReturnThread()
{
	CurrentProcess->Kill();
	if(CurrentProcess->ThreadID == 0)
	{
		CurrentProcess->OwnerProcess->Kill();
	}
	YieldCPU();
}

void Mutex::Lock()
{
	bool Locked = __sync_bool_compare_and_swap(&InUse, false, true);
	while(!Locked)
	{
		YieldCPU();
		Locked = __sync_bool_compare_and_swap(&InUse, false, true);
	}
	CurrentThreadID = CurrentProcess->ThreadID;
}

void Mutex::Unlock()
{
	if(CurrentThreadID == CurrentProcess->ThreadID)
	{
		CurrentThreadID = 0;
		InUse = false;
	}
}

void YieldCPU()
{
	asm("int $0x50");
}

//MUTEX CODE FOR GCC __sync_val_compare_and_swap (type *ptr, type oldval type newval, ...)

void CriticalRegion::Lock()
{
	RegionMutex.Lock();
}

void CriticalRegion::Unlock()
{
	RegionMutex.Unlock();
}