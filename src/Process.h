extern "C" void SwitchProcesses();
extern "C" void InitThread();
extern tss_entry TSS;

extern "C" void SwitchASM(long** OldRSP, long** NewRSP, long* NewCR3);
extern "C" void StartASM(long* NewRSP, long* NewCR3);

void* Malloc(unsigned long);
void ReturnThread();
extern "C" void BeginThread();

struct MBlockHeader 
{
	unsigned char PrevUsage;
	unsigned long PrevSize;
	unsigned long NextSize;
	unsigned char NextUsage;
} __attribute__((packed));

class Process;

//THREAD STATES VALUES
#define THREADSTATE_AVAILABLE 0
#define THREADSTATE_READY 1
#define THREADSTATE_RUNNING 2
#define THREADSTATE_BLOCKED 3

class Thread
{
	public:
	long Test = 0xDEADBEEF;
	char Type; //0 = Normal; 1 = Quick
	char State; 
	long* TSSRSP;
	long* TSSRBP;
	unsigned char ThreadID;
	long Duration, MaxDuration;
	PageFile* Page;
	Process* OwnerProcess;
	Thread* NextThread;
	Thread* NextThreadMutex = 0;
	Thread(void*, Process*, PageFile*, long);
	Thread(void*, Process*, PageFile*, long, int);
	Thread();
	void Start();
	void Block();
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
	unsigned char ProcessID;
	Process();
	Process(void*, const char*, unsigned char);
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
Thread* RoundRobinThread;

Process ProcessList[256];


template <typename ListType, unsigned int S> class FIFOList
{
	int Head, Tail;
	int Size = S;
	bool SameCycle;
	ListType List[S];
	ListType Read();
	void Write(ListType);
	bool Writable();
	bool Readable();
};

class Mutex
{
	protected:
	bool InUse; // 0 if free or ID of current thread using it
	bool Editing;
	Thread* CurrentThreadMutex;
	public:
	void Lock();
	void Unlock();
	Thread* QueueStart = (Thread*)0;
	Thread* QueueEnd = (Thread*)0;
};