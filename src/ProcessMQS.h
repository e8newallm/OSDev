extern "C" void SwitchProcesses();
extern "C" void InitThread();
extern tss_entry TSS;

extern "C" void SwitchASM(long** OldRSP, long** NewRSP, long* NewCR3);
extern "C" void StartASM(long* NewRSP, long* NewCR3);

void* Malloc(unsigned long);
void ReturnThread();
extern "C" void BeginThread();
void* Malloc(unsigned long);
void Free(void*);
void Scheduling();

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

#define THREADTYPE_UI 0
#define THREADTYPE_NORM 1
#define THREADTYPE_BACK 2

class Thread
{
	public:
	long Test = 0xDEADBEEF;
	char Type; //0 = UI; 1 = Normal; 2 = Background
	char State; 
	long* TSSRSP;
	long* TSSRBP;
	unsigned char ThreadID;
	long Duration, MaxDuration;
	PageFile* Page;
	Process* OwnerProcess;
	long LastUsage;
	Thread* NextThread;
	Thread* NextThreadMutex = 0;
	Thread* WaitingEndQueue = 0; //Waiting for thread to end
	Thread* WaitingEndQueueNext = 0; //Next thread waiting for the same thread to end
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
	Process(void*, const char*, unsigned char, bool);
	int Thread_Create(void*, int);
	void Start();
	void Kill();
	void StartThread(int);
	Thread ThreadList[256];
	char ProcessName[256];
	Thread* GetThread(int);
};

Thread* CurrentThread;
Thread* UIThread;
Thread* NormalThread;
Thread* BackgroundThread;
int CurrentThreadDuration;

volatile long UIThreadPeriod = 500;
volatile long NormalThreadPeriod = 450;
volatile long BackgroundThreadPeriod = 50;

volatile long UIThreadDuration = 0;
volatile long NormalThreadDuration = 0;
volatile long BackgroundThreadDuration = 0;

volatile long* CurrentThreadPeriod;
volatile long* CurrentPeriodDuration;

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
