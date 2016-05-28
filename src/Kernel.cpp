#define ProcessPBS
//#define ProcessQ
//#define ProcessRR
//#define ProcessMQS

long TimeSinceStart;
char TempStack[0x1000];
bool Testing = false;

void Kernel_Panic(const char*);
#include "E820.h"
#include "Miscellaneous/Miscellaneous.h"
#include "Memory/MemoryMap.h"
#include "Memory/Paging.h"

#ifdef ProcessQ
#include "Process.h"
#endif
#ifdef ProcessPBS
#include "ProcessPBS.h"
#endif
#ifdef ProcessRR
#include "ProcessRR.h"
#endif
#ifdef ProcessMQS
#include "ProcessMQS.h"
#endif

#include "Interrupts/IDT.h"
#include "Definitions.h"
#include "HPET.h"
#include "Serial.h"

#include "IO.cpp"
#include "Miscellaneous/Miscellaneous.cpp"
#include "Serial.cpp"
#include "Memory/MemoryMap.cpp"
#include "Memory/Paging.cpp"

#include "Keyboard.cpp"

#ifdef ProcessRR
#include "ProcessRR.cpp"
#endif
#ifdef ProcessQ
#include "Process.cpp"
#endif
#ifdef ProcessPBS
#include "ProcessPBS.cpp"
#endif
#ifdef ProcessMQS
#include "ProcessMQS.cpp"
#endif

#include "Memory/Malloc.cpp"

#include "Video/Video.cpp"
#include "Video/NewVideo.cpp"

#include "Interrupts/Exceptions.cpp"
#include "Interrupts/IDT.cpp"

#include "KernelPanic.cpp"
#include "BasicFunctions.cpp"

int Header[12] __attribute__((section (".Multiboot"))) = 
{
	0x1BADB002, 0x00000004, -(0x1BADB002 + 0x00000004),
	0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000,
	1024, 768, 24
};
void* KernelMemoryEnd = (void*)0x1FFFFFF;
char* StackBase = (char*)0x01F0000;
char extern KernelStart;
char extern KernelEnd;
tss_entry TSS;
extern TSS_entry_bits_64 GDTTSS;
extern gdt_entry_bits GDTData;
long* IDTPos;
multiboot_info_t* mbd;
char* BDA = (char*)0x400;
PageFile KernelMem; // The kernel memory

#include "KernelInterrupts.cpp"
#ifdef ProcessRR
#include "TestProgramsRR.cpp"
#endif
#ifdef ProcessQ
#include "TestProgramsQ.cpp"
#endif
#ifdef ProcessPBS
#include "TestProgramsPBS.cpp"
#endif
#ifdef ProcessMQS
#include "TestProgramsMQS.cpp"
#endif
#include "KernelProcesses.cpp"


/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX"); 
	CLI();
	//Setup Serial ports
	Serial.Setup(BDA);
	SerialLog = SerialQueue();
	//Setting up memory map
	PhysMemory.Initialise(mbd, (MemorySeg*)&KernelEnd, 0x1000);
	//Setting up IDT
	IDTPos = (long*)PhysMemory[PhysMemory.Size] + PhysMemory.MemorySegSize;
	IDT = (IDTStruct*)IDTPos;
	for(char* i = (char*)IDT; (unsigned long)i < (unsigned long)IDT + sizeof(IDTStruct); i++)
	{
		*i = 0;
	}
	IDT->Pointer.Limit = 0xFF * sizeof(IDTDescr) - 1;
	IDT->Pointer.Base = (long*)IDT;
	Pointer = (long*)&(IDT->Pointer);
	PICMapping_Init(0x20, 0x28); //Initialise the PICs, Master mapped to 0x20 - 0x27, Slave mapped to 0x28 - 0x30
	
	//Finding the ACPI tables
	RSDPDescriptor* RSDPSearch = (RSDPDescriptor*)0x10;
	while(strcmpl((char*)RSDPSearch, "RSD PTR ", 8))
	{
		RSDPSearch = (RSDPDescriptor*)((long)RSDPSearch + 0x10);
	}
	
	//Setting the exception gates
	SetGate(0x00, &Exc0, 0b11101110);
	SetGate(0x01, &Exc1, 0b11101110);
	SetGate(0x02, &Exc2, 0b11101110);
	SetGate(0x03, &Exc3, 0b11101110);
	SetGate(0x04, &Exc4, 0b11101110);
	SetGate(0x05, &Exc5, 0b11101110);
	SetGate(0x06, &Exc6, 0b11101110);
	SetGate(0x07, &Exc7, 0b11101110);
	SetGate(0x08, &Exc8, 0b11101110);
	SetGate(0x0A, &ExcA, 0b11101110);
	SetGate(0x0B, &ExcB, 0b11101110);
	SetGate(0x0C, &ExcC, 0b11101110);
	SetGate(0x0D, &ExcD, 0b11101110);
	SetGate(0x0E, &ExcE, 0b11101110);
	SetGate(0x10, &Exc10, 0b11101110);
	SetGate(0x11, &Exc11, 0b11101110);
	SetGate(0x12, &Exc12, 0b11101110);
	SetGate(0x14, &Exc14, 0b11101110);
	SetGate(0x1E, &Exc1E, 0b11101110);
	
	SetGate(0x20, &SystemTimerInt, 0b11101110);
	SetGate(0x21, &KeyboardInt, 0b11101110);
	
	SetGate(0x50, (long*)&SwitchThread, 0b11101110);
	SetGate(0x51, (long*)&SysCall, 0b11101110);
	TimeSinceStart = 0;
	
	Output8(0x43, 0b00110000); //Set counter 0 to Interrupt On Terminal Count 
	__asm__("PUSH %RAX; MOV (Pointer), %eax; LIDT (%eax);sti; POP %RAX");
	
	//Setting up TSS
	unsigned long Base = (unsigned long) &TSS;
	unsigned long Limit = sizeof(tss_entry);
	GDTTSS.base_low = Base&0xFFFFFF;
	GDTTSS.limit_low = Limit&0xFFFF;
	GDTTSS.always_0 = 0; //indicate it is a TSS
	GDTTSS.Type = 0b1001;
	GDTTSS.DPL = 3; //same meaning
	GDTTSS.present = 1; //same meaning
	GDTTSS.limit_high = (Limit&0xF0000)>>16; //isolate top nibble
	GDTTSS.available = 0;
	GDTTSS.always_0_2 = 0; //same thing
	GDTTSS.gran = 0; //so that our computed GDT limit is in bytes, not pages
	GDTTSS.base_high = (Base&0xFF000000)>>24; //isolate top byte.
	GDTTSS.Base_top = Base>>32;
	memset(&TSS, 0, sizeof(tss_entry));
	TSS.RSP0 = (unsigned long)StackBase;
	__asm__("PUSH %RAX; MOV $0x28, %AX; LTR %AX; POP %RAX");
	for(MemorySeg* Position = PhysMemory.FindPhyAddr((void*)0x0); Position <= PhysMemory.FindPhyAddr(KernelMemoryEnd); Position++)
	{
		PhysMemory.UsePhyAddr(Position, MEMORYSEG_LOCKED); 
	}
	ModelPaging.Pages = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
	unsigned long FinalSeg = (PhysMemory.EOM() - 1)->BaseAddress;
	for(unsigned long i = 0; i < FinalSeg; i += 0x1000)
	{
		unsigned long AlignedPhyAddr = i & 0xFFFFFFFFFFFFF000;
		unsigned short PML4Index = CONVERTFROMPML4(i) + 256;
		if(ModelPaging.Pages[PML4Index] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			ModelPaging.Pages[PML4Index] = (long)NewTable | 3; //Temporary
		}
		long* PDPTTable = (long*)((long)ModelPaging.Pages[PML4Index] & 0x7FFFFFFFFF000);
		unsigned short PDPTIndex = CONVERTFROMPDPT(i);
		if(PDPTTable[PDPTIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			PDPTTable[PDPTIndex] = (long)NewTable | 3; //Temporary
		}
		long* PDTable = (long*)((long)PDPTTable[PDPTIndex] & 0x7FFFFFFFFF000);
		unsigned short PDIndex = CONVERTFROMPD(i);
		if(PDTable[PDIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			PDTable[PDIndex] = (long)NewTable | 3; //Temporary
		}
		long* PETable = (long*)((long)PDTable[PDIndex] & 0x7FFFFFFFFF000);
		unsigned short PTIndex = CONVERTFROMPT(i);
		PETable[PTIndex] = (long)AlignedPhyAddr | 3;
	}
	PageFile TempPage;
	TempPage.Pages = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
	for(int i = 0; i < 512; i++)
		TempPage.Pages[i] = ModelPaging.Pages[i];
	for(unsigned long i = 0; i < ProcessMemStart; i += 0x1000)
	{
		unsigned long AlignedPhyAddr = i & 0xFFFFFFFFFFFFF000;
		unsigned char PML4Index = CONVERTFROMPML4(i);
		long Temp = TempPage.Pages[PML4Index];
		if(Temp == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr();
			TempPage.Pages[PML4Index] = (long)NewTable | 3; //Temporary
		}
		long* PDPTTable = (long*)((long)TempPage.Pages[PML4Index] & 0x7FFFFFFFFF000);
		unsigned short PDPTIndex = CONVERTFROMPDPT(i);
		if(PDPTTable[PDPTIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr();
			PDPTTable[PDPTIndex] = (long)NewTable | 3; //Temporary
		}
		long* PDTable = (long*)((long)PDPTTable[PDPTIndex] & 0x7FFFFFFFFF000);
		unsigned short PDIndex = CONVERTFROMPD(i);
		if(PDTable[PDIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr();
			PDTable[PDIndex] = (long)NewTable | 3; //Temporary
		}
		long* PETable = (long*)((long)PDTable[PDIndex] & 0x7FFFFFFFFF000);
		unsigned short PTIndex = CONVERTFROMPT(i);
		PETable[PTIndex] = (long)AlignedPhyAddr | 3;
	}
	TempPage.Activate();
	
	//Graphics setup
	GUI = Video((vbe_mode_info_struct*)((long)mbd->vbe_mode_info));
	unsigned long End = 0xA00000;
	GUI.SecondFrameAddress = (unsigned char*)End;
	
	//Setting up processes
	for(long i = 0; i < 256; i++)
	{
		ProcessList[i].Available = true;
	}
	int ID = Process_Make((void*)&SystemIdle, "Idle Process", 1);
	CurrentThread = GetProcess(ID)->GetThread(0);
	#ifdef ProcessQ
	CurrentThread->NextThread = CurrentThread;
	RoundRobinThread = CurrentThread;
	#endif
	#ifdef ProcessRR
	CurrentThread->NextThread = CurrentThread;
	#endif
	#ifdef ProcessMQS
	CurrentThread->NextThread = CurrentThread;
	CurrentThreadPeriod = &BackgroundThreadPeriod;
	CurrentPeriodDuration = &BackgroundThreadDuration;
	#endif
	GetProcess(ID)->Start();
	ID = Process_Make((void*)&Graphics, "Graphics Process", 1);
	GetProcess(ID)->Start();
	ID = Process_Make((void*)&SerialWrite, "Log writer", 1);
	GetProcess(ID)->Start();
	ID = Process_Make((void*)&TaskManager, "Task Manager", 1);
	GetProcess(ID)->Start();
	ID = Process_Make((void*)&TestAutomation, "Test Automation", 1);
	GetProcess(ID)->Start();
	ID = Process_Make((void*)&Textbox, "Text ewqdwqw", 1);
	GetProcess(ID)->Start();
	//Enables multitasking and PIT(As well as other IRQs)
	Output8(PICM_Dat, 0xFC); //0xFC
	Output8(PICS_Dat, 0xFF);
	StartProcesses();
}
/////////////////////KERNEL END///////////////////////////
