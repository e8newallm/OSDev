extern "C" void ProcessSwitch(long* OldRSP, long* NewRSP, long* NewCR3);
void SwitchProcesses();
long NextProcessID;

#define MBlockHeader_Free 0
#define MBlockHeader_InUse 1
#define MBlockHeader_Start (unsigned char)254
#define MBlockHeader_End (unsigned char)255

#define ThreadStackStart 0xFFF01000

void* Malloc(unsigned long);
void ReturnThread();

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
	bool Available;
	long* RSP;
	int ThreadID;
	long Duration, MaxDuration;
	PageFile* Page;
	Process* OwnerProcess;
	const char* ProcessName;
	Thread* NextProcess;
	bool Killed;
	Thread(void*, long, Process*, PageFile*, long);
	Thread(void*, long, Process*, long);
	Thread();
	void Start();
	void Kill();
};

class Process
{
public:
	bool Available;
	long* RSP;
	long Duration, MaxDuration;
	PageFile Page;
	void* MemStart;
	MBlockHeader* StartBlock;
	MBlockHeader* EndBlock;
	bool Killed;
	Process();
	Process(void*, long, const char*,Process*);
	int CreateThread(void*, long);
	void Start();
	void Kill();
	void StartThread(int);
	Thread ThreadList[255];
	long NextStack;
	const char* ProcessName;
	
};

Thread* CurrentProcess;
Process ProcessList[255];

int CreateProcess(void* Main, long Duration, const char* Name)
{
	CLI();
	for(long i = 0; i < 255; i++)
	{
		if(ProcessList[i].Available)
		{
			ProcessList[i] = Process(Main, Duration, Name, (ProcessList+i));
			STI();
			return i;
		}
	}
	Kernel_Panic("OH SHIT NO PROCESSES LEFT");
}

Process GetProcess(int ID)
{
	return ProcessList[ID];
}

/////////////////////////PROCESS CODE///////////////////////////////////////

Process::Process(void* Main, long DurationMax, const char* Name, Process* VariablePlace)
{
	Available = false;
	ProcessName = Name;
	Duration = 0;
	MaxDuration = DurationMax;
	Page = PageFile();
	Page.SetupStartMemory();
	NextStack = 0xFFF01000;
	MemStart = (void*)(ProcessMemStart);
	Page.MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(ProcessMemStart));
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	Page.Activate();
	MBlockHeader* BlockSetup = (MBlockHeader*)(ProcessMemStart);
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
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
	ThreadList[0] = Thread(Main, DurationMax, VariablePlace, &Page, 0);
	NextStack += 0x1000;
	Killed = false;
}

Process::Process()
{
	Available = true;
}

void SwitchProcesses()
{
	CLI();
	asm("PUSH %RAX; PUSH %RCX; PUSH %RBP; PUSH %RBX; PUSH %RDX; PUSH %RSI; PUSH %RDI; PUSH %R8; PUSH %R9; PUSH %R10; PUSH %R11; PUSH %R12; PUSH %R13; PUSH %R14; PUSH %R15; PUSHF");
	if(Multitasking)
	{
		CurrentProcess->Duration = 0;
		Thread* Next = CurrentProcess->NextProcess;
		while(Next->Killed)
		{
			Next->Available = true;
			Next = Next->NextProcess;
			CurrentProcess->NextProcess = Next;
		}
		long* CurrentRSPAddress = (long*)&(CurrentProcess->RSP);
		long* NextRSP = Next->RSP;
		long* NextPage = (Next->Page)->Pages;
		CurrentProcess = CurrentProcess->NextProcess;
		CurrentProcess->Duration = 0;
		
		asm volatile("MOV %%RSP, (%0); MOV %2, %%CR3; MOV %1, %%RSP;"
				 : : "r"(CurrentRSPAddress), "r"(NextRSP), "r"(NextPage));
		
		//ProcessSwitch(CurrentRSPAddress, NextRSP, NextPage);
		//Serial.WriteString(0x1, "\r\nShit's swapped yo");
	}
	Output8(0x40, 0x4E); //Set lower byte 0x4E
	Output8(0x40, 0x17); //set higher byte 0x17
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RBP; POP %RCX; POP %RAX");
	STI();
}

void StartProcesses()
{
	CLI();
	if(Multitasking)
	{
		CurrentProcess->Duration = 0;
		long* NextRSP = CurrentProcess->RSP;
		long* NextPage = (CurrentProcess->Page)->Pages;
		asm volatile(" MOV %1, %%CR3; MOV %0, %%RSP;"
				 : :"r"(NextRSP), "r"(NextPage));
		
		//ProcessSwitch(CurrentRSPAddress, NextRSP, NextPage);
	}
	Output8(0x40, 0x4E); //Set lower byte 0x4E
	Output8(0x40, 0x17); //set higher byte 0x17
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RDI; POP %RSI; POP %RDX; POP %RBX; POP %RBP; POP %RCX; POP %RAX");
	STI();
}

void Process::Start()
{
	ThreadList[0].Start();
}

void Process::StartThread(int ThreadID)
{
	Serial.WriteString(0x1, "\r\nStarting Thread ID ");
	Serial.WriteLongHex(0x1, ThreadID);
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

int Process::CreateThread(void* Main, long Duration)
{
	for(long i = 0; i < 255; i++)
	{
		if(ThreadList[i].Available)
		{
			ThreadList[i] = Thread(Main, Duration, this, &Page, i);
			return i;
		}
	}
	return -1;
}

unsigned long ThreadMain, ThreadBP;

Thread::Thread(void* Main, long DurationMax, Process* OwnerProcessIn, PageFile* PageIn, long IDThread)
{
	Available = false;
	ThreadID = IDThread;
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	OwnerProcess = OwnerProcessIn;
	Page = PageIn;
	char* Stack = (char*)(ThreadStackStart + (IDThread * 0x1000));
	Page->MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(Stack));
	RSP = (long*)(Stack + (0x1000 - (8 * 18))); //Add starting values to stack for ProcessSwitch to switch in
	ThreadBP = (long)(Stack + 0xFFF);
	ThreadMain = (long)Main;
	Page->Activate();
	RSP[0] = 0x202; //EFLAG starting value (IF=1)
	RSP[16] = ThreadMain; //Code start address
	RSP[13] = ThreadBP;
	RSP[17] = (long)&ReturnThread; //End thread address
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
	Page = &(OwnerProcess->Page);
	Killed = false;
}

Thread::Thread(void* Main, long DurationMax, Process* OwnerProcessIn, long IDThread)
{
	Available = false;
	ThreadID = IDThread;
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	OwnerProcess = OwnerProcessIn;
	Page = &(OwnerProcess->Page);
	char* Stack = (char*)(ThreadStackStart + (IDThread * 0x1000));
	Page->MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(Stack));
	RSP = (long*)(Stack + (0x1000 - (8 * 18))); //Add starting values to stack for ProcessSwitch to switch in
	ThreadBP = (long)(Stack + 0xFFF);
	ThreadMain = (long)Main;
	Page->Activate();
	RSP[0] = 0x202; //EFLAG starting value (IF=1)
	RSP[16] = ThreadMain; //Code start address
	RSP[13] = ThreadBP;
	RSP[17] = (long)&ReturnThread; //End thread address
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
	Killed = false;
}

void Thread::Start()
{
	CLI();
	NextProcess = CurrentProcess->NextProcess;
	CurrentProcess->NextProcess = this;
	STI();
	SwitchProcesses();
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

void ReturnThread()
{
	CLI();
	CurrentProcess->Kill();
	if(CurrentProcess->ThreadID == 0)
	{
		CurrentProcess->OwnerProcess->Kill();
	}
	STI();
	SwitchProcesses();
}

//MUTEX CODE FOR GCC __sync_lock_test_and_set (type *ptr, type value, ...)