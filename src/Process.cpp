extern long ProcessSwapData;
extern "C" void ProcessSwitch(long* OldRSP, long* NewRSP, long* NewCR3);

#define MBlockHeader_Free 0
#define MBlockHeader_InUse 1
#define MBlockHeader_Start (unsigned char)254
#define MBlockHeader_End (unsigned char)255

struct MBlockHeader 
{
	unsigned char PrevUsage;
	long PrevSize;
	long NextSize;
	unsigned char NextUsage;
} __attribute__((packed));

class Process
{
public:
	long* RSP;
	long Duration, MaxDuration;
	PageFile Page;
	Process* NextProcess;
	void* MemStart;
	MBlockHeader* StartBlock;
	MBlockHeader* EndBlock;
	bool Killed;
	Process();
	Process(void*, void*, long);
	void SaveProcess();
	void LoadProcess();
	void Start();
	void Kill();
};

Process* CurrentProcess;
bool Multitasking = false;

Process::Process(void* Main, void* Stack, long DurationMax)
{
	Duration = 0;
	MaxDuration = DurationMax;
	Page = PageFile();
	Page.SetupStartMemory();
	unsigned long PageStack = ProcessMemStart;
	unsigned long CurrentPage;
	MemStart = (void*)(ProcessMemStart + 0x1000);
	Page.MapAddress((unsigned long)Stack, PageStack);
	Page.MapAddress((unsigned long)PhysMemory.UseFreePhyAddr(), (unsigned long)(ProcessMemStart + 0x1000));
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	Page.Activate();
	MBlockHeader* BlockSetup = (MBlockHeader*)(ProcessMemStart + 0x1000);
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
	RSP = (long*)((char*)PageStack + (0xFFF - (8 * 13)));
	RSP[13] = (long)Main;
	RSP[5] = (long)((char*)PageStack + 0xFFF);
	
	asm volatile("MOV %0, %%CR3" : : "r"(CurrentPage));
	
	Killed = false;
}

Process::Process()
{
}

void Process::Start()
{
	NextProcess = CurrentProcess->NextProcess;
	CurrentProcess->NextProcess = this;
}

extern "C" void SwitchProcesses()
{
	asm("PUSH %RDI; PUSH %RSI; PUSH %RDX");
	//Serial.WriteString(0x1, "\r\nSwap process");
	if(Multitasking)
	{
		Process* Next = CurrentProcess->NextProcess;
		while(Next->Killed)
		{
			Next = Next->NextProcess;
			CurrentProcess->NextProcess = Next;
		}
		long* CurrentRSPAddress = (long*)&(CurrentProcess->RSP);
		long* NextRSP = Next->RSP;
		long* NextPage = (Next->Page).Pages;
		CurrentProcess = CurrentProcess->NextProcess;
		CurrentProcess->Duration = 0;
		ProcessSwitch(CurrentRSPAddress, NextRSP, NextPage);
		asm("POP %RDX; POP %RSI; POP %RDI");
	}
}

void Process::Kill()
{
	Killed = true;
	//TODO: Add freeing of memory and closing of process if the current active one
}