long TimeSinceStart;
long TimeSinceStartPart;
char TempStack[0x1000];
bool Multitasking = false;
bool Testing = false;
#define CLI() __asm__("CLI");
#define STI() __asm__("STI");
unsigned char Hex[17] = "0123456789ABCDEF";
unsigned char Dec[11] = "0123456789";
#include "IO.cpp"
#include "Serial.cpp"
#include "Miscellaneous/Miscellaneous.cpp"
#include "E820.h"
#include "Memory/MemoryMap.cpp"
#include "Memory/Paging.cpp"
//#include "BasicFunctions.cpp"

#include "Keyboard.cpp"
#include "Process.cpp"
#include "Memory/Malloc.cpp"

#include "Video/Video.cpp"

#include "Interrupts/Exceptions.cpp"
#include "Interrupts/IDT.cpp"

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
Process Halt, Test, GraphDriver;
PageFile KernelMem; // The kernel memory
__attribute__((noinline)) volatile void Graphics()
{
	for(unsigned long i = 0; i <= GUI.BytesPerLine * GUI.Height; i += 0x1000)
	{
		GraphDriver.Page.MapAddress(((unsigned long)GUI.FrameAddress + i), ((long)GraphDriver.MemStart) + i);
	}
	//Serial.WriteString(0x1, "\r\nDone");
	GUI.FrameAddress = (unsigned char*)((long)GraphDriver.MemStart);
	char shade = 255;
	char* MainFrame = (char*)GUI.FrameAddress;
	char* NewFrame = (char*)GUI.SecondFrameAddress;
	//Serial.WriteString(0x1, "\r\nWhile(1)");
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nUpdating Graphics");
		for(int i = 0; i < GUI.Height; i++)
		{
			for(int j = 0; j < (GUI.Width*GUI.Depth); j++)
			{
				int pos = (i * GUI.BytesPerLine) + j;
				MainFrame[pos] = NewFrame[pos];
			}
		}
		SwitchProcesses();
	}
}

__attribute__((noinline)) volatile void TestProcess()
{
	//Serial.WriteString(0x1, "\r\nGraphics Loaded!");
	int x = 214, y = 532;
	int xVel = 15, yVel = 18;
	long Width = GUI.Width, Height = GUI.Height;
	long Time = TimeSinceStart;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		GUI.DrawPixel(x, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x, y+1, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y+1, (char)(128), (char)(50), (char)(75));
		STI();
		//while(TimeSinceStart - Time < 500)
		//{
			SwitchProcesses();
		//}
		CLI();
		Time = TimeSinceStart;
		GUI.DrawPixel(x, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x, y+1, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y+1, (char)(0), (char)(0), (char)(0));
		//DrawString("Test string", 11, x, y);
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x > GUI.Width)
		{
			x = GUI.Width;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y > GUI.Height)
		{
			y = GUI.Height;
			yVel = -yVel;
		}
	}
}

__attribute__((noinline)) volatile void TestProcessTwo()
{
	//Serial.WriteString(0x1, "\r\nGraphics Loaded!");
	int x = 214, y = 532;
	int xVel = -15, yVel = -18;
	long Width = GUI.Width, Height = GUI.Height;
	long Time = TimeSinceStart;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		GUI.DrawPixel(x, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x, y+1, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y+1, (char)(128), (char)(50), (char)(75));
		STI();
		//while(TimeSinceStart - Time < 500)
		//{
			SwitchProcesses();
		//}
		CLI();
		Time = TimeSinceStart;
		GUI.DrawPixel(x, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x, y+1, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y+1, (char)(0), (char)(0), (char)(0));
		//DrawString("Test string", 11, x, y);
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x > GUI.Width)
		{
			x = GUI.Width;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y > GUI.Height)
		{
			y = GUI.Height;
			yVel = -yVel;
		}
	}
}

__attribute__((noinline)) volatile void TestProcessThree()
{
	int x = 400, y = 700;
	int xVel = -15, yVel = -2;
	long Width = GUI.Width, Height = GUI.Height;
	long Time = TimeSinceStart;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		GUI.DrawPixel(x, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x, y+1, (char)(128), (char)(50), (char)(75));
		GUI.DrawPixel(x+1, y+1, (char)(128), (char)(50), (char)(75));
		STI();
		//while(TimeSinceStart - Time < 500)
		//{
			SwitchProcesses();
		//}
		CLI();
		Time = TimeSinceStart;
		GUI.DrawPixel(x, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x, y+1, (char)(0), (char)(0), (char)(0));
		GUI.DrawPixel(x+1, y+1, (char)(0), (char)(0), (char)(0));
		//DrawString("Test string", 11, x, y);
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x > GUI.Width)
		{
			x = GUI.Width;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y > GUI.Height)
		{
			y = GUI.Height;
			yVel = -yVel;
		}
	}
}

__attribute__((noinline)) volatile void SystemIdle() 
{
	GraphDriver.Start();
	CreateProcess((void*)&TestProcess, 10, "Test Program");
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nIdling");
		//Serial.WriteString(0x1, "\r\nSwapping");
		SwitchProcesses();
	}
}

/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX"); 
	CLI();
	//Setup Serial ports
	Serial.Setup(BDA);
	Serial.WriteString(0x1, "Serial started!\r\n");
	//Setting up memory map
	PhysMemory.Initialise(mbd, (MemorySeg*)&KernelEnd, 0x1000);
	MemorySeg* LoopChk = PhysMemory.FindPhyAddr(&KernelEnd);
	MemorySeg* KernelPointer;
	//Setting up IDT
	IDTPos = (long*)PhysMemory[PhysMemory.Size] + PhysMemory.MemorySegSize;
	IDT = (IDTStruct*)IDTPos;
	for(char* i = (char*)IDT; (long)i < (long)IDT + sizeof(IDTStruct); i++)
	{
		*i = 0;
	}
	IDT->Pointer.Limit = 0xFF * sizeof(IDTDescr) - 1;
	IDT->Pointer.Base = (long*)IDT;
	Pointer = (long*)&(IDT->Pointer);
	PICMapping_Init(0x20, 0x28); //Initialise the PICs, Master mapped to 0x20 - 0x27, Slave mapped to 0x28 - 0x30
	
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
	TimeSinceStart = 0;
	
	Output8(0x43, 0b00110000); //Set counter 0 to Interrupt On Terminal Count 
	Output8(0x40, 0x4E); //Set lower byte 0x4E
	Output8(0x40, 0x17); //set higher byte 0x17
	__asm__("MOV (Pointer), %eax; LIDT (%eax);sti");
	
	//Setting up TSS
	unsigned long Base = (unsigned long) &TSS;
	unsigned long Limit = sizeof(tss_entry);
	Serial.WriteString(0x1, "\r\nSetting up TSS. Size: ");
	Serial.WriteLong(0x1, Limit);
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
	Serial.WriteString(0x1, "\r\nSet up TSS");
	memset(&TSS, 0, sizeof(tss_entry));
	
	TSS.RSP0 = (unsigned long)StackBase; // ROFL THIS NEEDS CHANGING CAUSE DERP TWO SYS CALLS AT ONCE BREAKS IT
	__asm__("MOV $0x28, %AX; LTR %AX");
	for(MemorySeg* Position = PhysMemory.FindPhyAddr((void*)0x0); Position <= PhysMemory.FindPhyAddr(KernelMemoryEnd); Position++)
	{
		PhysMemory.UsePhyAddr(Position, MEMORYSEG_LOCKED); 
	}
	Paging.Pages = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
	long FinalSeg = (PhysMemory.EOM() - 1)->BaseAddress;
	for(unsigned long i = 0; i < FinalSeg; i += 0x1000)
	{
		unsigned long AlignedPhyAddr = i & 0xFFFFFFFFFFFFF000;
		unsigned char PML4Index = CONVERTFROMPML4(i);
		if(Paging.Pages[PML4Index] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			Paging.Pages[PML4Index] = (long)NewTable | 7; //Temporary
		}
		long* PDPTTable = (long*)((long)Paging.Pages[PML4Index] & 0x7FFFFFFFFF000);
		unsigned short PDPTIndex = CONVERTFROMPDPT(i);
		if(PDPTTable[PDPTIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			PDPTTable[PDPTIndex] = (long)NewTable | 7; //Temporary
		}
		long* PDTable = (long*)((long)PDPTTable[PDPTIndex] & 0x7FFFFFFFFF000);
		unsigned short PDIndex = CONVERTFROMPD(i);
		if(PDTable[PDIndex] == (long)0)
		{
			long* NewTable = (long*)PhysMemory.UseFreePhyAddr(MEMORYSEG_LOCKED);
			PDTable[PDIndex] = (long)NewTable | 7; //Temporary
		}
		long* PETable = (long*)((long)PDTable[PDIndex] & 0x7FFFFFFFFF000);
		unsigned short PTIndex = CONVERTFROMPT(i);
		PETable[PTIndex] = (long)AlignedPhyAddr | 7;
	}
	Paging.Activate();
	//Graphics setup
	GUI = Video((vbe_mode_info_struct*)((long)mbd->vbe_mode_info));
	unsigned long End = 0xA00000;
	GUI.SecondFrameAddress = (unsigned char*)End;
	Halt = Process((void*)&SystemIdle, 20, "Idle Process", &Halt);
	CurrentProcess = &(Halt.ThreadList[0]);
	CurrentProcess->NextProcess = CurrentProcess;
	GraphDriver = Process((void*)&Graphics, 100, "Graphics Process", &GraphDriver);
	//Enables multitasking and PIT(As well as other IRQs) and swaps to ring 3
	Output8(PICM_Dat, 0xFC);
	Output8(PICS_Dat, 0xFF);
	Multitasking = true;
	Serial.WriteString(0x1, "\r\nSwapping to ring 3");
	asm volatile("MOV $0x23, %AX; MOV %AX, %DS; MOV %AX, %ES; MOV %AX, %FS; MOV %AX, %GS; MOV %ESP, %EAX; PUSH $0x23; PUSH %RSP; PUSHF; PUSH $0x1B; PUSH $1f; IRETQ; 1:CLI");
	Serial.WriteString(0x1, "\r\nStarting processes");
	StartProcesses();
}
/////////////////////KERNEL END///////////////////////////