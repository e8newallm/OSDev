extern long ProcessSwapData;
extern "C" void ProcessSwitch(long* OldRSP, long* NewRSP, long* NewCR3);
class Process
{
public:
	long* RSP;
	long Duration, MaxDuration;
	PageFile Page;
	Process* NextProcess;
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
	unsigned long PageStack = Page.GetFreeAddress();
	unsigned long CurrentPage;
	Page.MapAddress((unsigned long)Stack, PageStack);
	asm volatile("MOV %%CR3, %0" : "=r"(CurrentPage));
	RSP = (long*)((char*)PageStack + 0xFC7);
	Page.Activate();
	RSP[6] = (long)Main;
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
	//Serial.WriteString(0x1, "\r\nRotating processes");
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
		STI();
		//Serial.WriteString(0x1, "\r\nProcess switch RSP: ");
		//Serial.WriteLongHex(0x1, (long)NextRSP);
		ProcessSwitch(CurrentRSPAddress, NextRSP, NextPage);
	}
}

void Process::Kill()
{
	Killed = true;
	//TODO: Add freeing of memory and closing of process if the current active one
}