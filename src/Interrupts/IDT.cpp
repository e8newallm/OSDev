#define PICM_Com 0x20
#define PICM_Dat 0x21
#define PICS_Com 0xA0
#define PICS_Dat 0xA1

#define PIC_EOI 0x20

inline void PushAll()
{
	asm("PUSH %RAX; PUSH %RCX; PUSH %RBP; PUSH %RBX; PUSH %RSI; PUSH %RDI; PUSH %RBP; PUSH %R8; PUSH %R9; PUSH %R10; PUSH %R11; PUSH %R12; PUSH %R13; PUSH %R14; PUSH %R15; PUSHF");
}

inline void PopAll()
{
	asm("POPF; POP %R15; POP %R14; POP %R13; POP %R12; POP %R11; POP %R10; POP %R9; POP %R8; POP %RBP; POP %RDI; POP %RSI; POP %RBX; POP %RBP; POP %RCX; POP %RAX");
}

struct __attribute__ ((packed)) IDTDescr
{
   short Offset1;
   short Selector;
   char Zero;
   char TypeAttr; 
   short Offset2;
   int Offset3;	
   int Zero2;
};

struct IDTPtr
{
    unsigned short Limit;
    long* Base;
} __attribute__((packed));


struct __attribute__((packed)) IDTStruct
{
	IDTDescr Entries[0xFF];
	IDTPtr Pointer;
};

long extern KeyboardInt, SystemTimerInt, ProcessSwitchInt;

long extern Exc0, Exc1, Exc2, Exc3, Exc4, Exc5,
			Exc6, Exc7, Exc8, ExcA, ExcB, ExcC,
			ExcD, ExcE, Exc10, Exc11, Exc12,
			Exc13, Exc14, Exc1E;
IDTStruct* IDT;
long* Pointer;

void PICEndInt(char IRQ)
{
	if(IRQ >= 8)
	{
		Output8(PICS_Com, PIC_EOI);
	}
	Output8(PICM_Com, PIC_EOI);
}

inline void Int22()
{
	__asm__("int $0x22\r\n");
}

void SetGate(char GateAddr, long* Offset, char Type)
{
	IDT->Entries[GateAddr].Selector = 0x8;
	IDT->Entries[GateAddr].TypeAttr = Type;
	IDT->Entries[GateAddr].Offset1 = (long)Offset & 0xFFFF;
	IDT->Entries[GateAddr].Offset2 = ((long)Offset >> 16) & 0xFFFF;
	IDT->Entries[GateAddr].Offset3 = (long)Offset >> 32;
	IDT->Entries[GateAddr].Zero = 0x0;
	IDT->Entries[GateAddr].Zero2 = 0x0;
}

void PICMapping_Init(char PICMOff, char PICSOff)
{
	Output8(PICM_Com, 0x11); //ICW1
	Output8(PICS_Com, 0x11); //ICW1
	
	Output8(PICM_Dat, PICMOff); //ICW2 - Offset
	Output8(PICS_Dat, PICSOff); //ICW2 - Offset
	
	Output8(PICM_Dat, 4); //ICW3 - Cascade
	Output8(PICS_Dat, 2); //ICW3 - Cascade
	
	Output8(PICM_Dat, 1); //ICW4 - Cascade
	Output8(PICS_Dat, 1); //ICW4 - Cascade
	
	Output8(PICM_Dat, 0xFF);
	Output8(PICS_Dat, 0xFF);
}

extern "C" void KeyboardInterrupt()
{
	PushAll();
	CLI();
	char PressedKey = Input8(0x60);
	bool Test;
	if(PressedKey < 0x58)
	{
		Test = PutKeyPress(ScanCodes1[PressedKey]);
	}
	if(!Test)
	{
		Kernel_Panic("\r\nKeypress queue full!");
	}
	PICEndInt((char)1);
	STI();
	PopAll();
}

extern "C" void SystemTimerInterrupt()
{
	PushAll();
	CLI();
	TimeSinceStart += 10;
	CurrentThreadDuration += 10;
	CurrentPeriodDuration += 10;
	short NewDuration = 0x2E9B;
	Output8(0x40, NewDuration&0xFF); //Set lower byte 0x4E
	Output8(0x40, NewDuration>>8); //set higher byte 0x17
	PICEndInt((char)0);
	if((!CurrentThreadPeriod && CurrentPeriodDuration >= NormalThreadPeriod) || (CurrentThreadPeriod && CurrentPeriodDuration >= QuickThreadPeriod))
	{
		if(!CurrentThreadPeriod)
			LastNormalThread = CurrentThread;
		else
			CurrentThread = LastNormalThread;
		CurrentThreadPeriod = !CurrentThreadPeriod;
		CurrentPeriodDuration = 0;
		SwitchProcesses();
	}
	else if(CurrentThreadDuration >= CurrentThread->Duration)
		SwitchProcesses();
	STI();
	PopAll();
}

extern "C" void SysCall();
extern "C" void SwitchThread();

extern "C" void SysCallSwitch()
{
	long RAX, RBX, RCX, RDX, R8; //RAX = FUNCTION
	asm volatile("MOV %%RAX, %0":"=m"(RAX) :: "%rax");
	asm volatile("MOV %%RBX, %0":"=m"(RBX) :: "%rbx");
	asm volatile("MOV %%RCX, %0":"=m"(RCX) :: "%rcx");
	asm volatile("MOV %%RDX, %0":"=m"(RDX) :: "%rdx");
	
	asm volatile("MOV %%R8, %0":"=m"(R8) :: "%r8");
	
	switch(RAX)
	{
		case PROCESS_MAKE_ID: //Create Process %RAX = PROCESS_MAKE_ID; %RBX = Main; %RCX = Name; Returns ID = %RAX
		{
			int ID = Process_Make((void*)RBX, (const char*)RCX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		case THREAD_MAKE_ID: //Create Thread %RAX = THREAD_MAKE_ID; %RBX = Main; %RCX = ProcessID; Returns ID = %RAX
		{
			int ID = GetProcess(RCX)->Thread_Create((void*)RBX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		case PAGE_MAP_ID: //Map virtual address %RAX = PAGE_MAP_ID; %RBX = Size; Returns nothing
		{
			AddVMemory(RBX);
			break;
		}
	}
}
