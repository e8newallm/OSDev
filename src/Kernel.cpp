long TimeSinceStart;
double TimeSinceStartPart;

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

char* StackBase = (char*)0x001F000;
char* StackBottom = (char*)0x001D000;
void* KernelMemoryEnd = (void*)0x1FFFFFF;
char extern KernelStart;
char extern KernelEnd;
long* IDTPos;
multiboot_info_t* mbd;
//SerialController Serial;
unsigned short MemoryModel;
char* BDA = (char*)0x400;
Process Halt, Test, GraphDriver;

__attribute__((noinline)) volatile void SystemIdle() 
{
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nIdling");
		__asm__("HLT");
	}
}

__attribute__((noinline)) volatile void Graphics()
{
	STI();
	Serial.WriteString(0x1, "\r\nMap main video memory. Size: ");
	Serial.WriteLongHex(0x1, (long)GUI.BytesPerLine * GUI.Height);
	for(unsigned long i = 0; i <= GUI.BytesPerLine * GUI.Height; i += 0x1000)
	{
		GraphDriver.Page.MapAddress(((unsigned long)GUI.FrameAddress + i), ((long)GraphDriver.MemStart) + i);
	}
	GUI.FrameAddress = (unsigned char*)((long)GraphDriver.MemStart);
	char shade = 255;
	char* MainFrame = (char*)GUI.FrameAddress;
	char* NewFrame = (char*)GUI.SecondFrameAddress;
	Serial.WriteString(0x1, "\r\nFrame Address: ");
	Serial.WriteLongHex(0x1, (long)GUI.FrameAddress);
	/*while(1)
	{
		//Serial.WriteString(0x1, "\r\nStart drawing");
		for(int i = 0; i < GUI.Height; i++)
		{
			for(int j = 0; j < GUI.Width*3; j++)
			{
				MainFrame[(i * GUI.BytesPerLine) + j] = shade;
				shade -= 5;
			}
		}
		//Serial.WriteString(0x1, "\r\nDone drawing");
	}*/
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
		/*for(int i = 0; i < End; i++)
		{
			MainFrame[i] = NewFrame[i];
		}*/
		SwitchProcesses();
	}
}

__attribute__((noinline)) volatile void TestProcess()
{
	STI();
	int x = 214, y = 532;
	int xVel = 15, yVel = 18;
	while(1)
	{
		//Serial.WriteString(0x1, "\r\nTesting");
		long Width = GUI.Width, Height = GUI.Height;
		GUI.DrawPixel(x, y, (char)(128+y), (char)(50+y), (char)(75+y));
		x++;
		y++;
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

/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX"); 
	
	//Setup Serial ports
	Serial.Setup(BDA);
	
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
	SetGate(0x00, &Exc0, 0b10001110);
	SetGate(0x01, &Exc1, 0b10001110);
	SetGate(0x02, &Exc2, 0b10001110);
	SetGate(0x03, &Exc3, 0b10001110);
	SetGate(0x04, &Exc4, 0b10001110);
	SetGate(0x05, &Exc5, 0b10001110);
	SetGate(0x06, &Exc6, 0b10001110);
	SetGate(0x07, &Exc7, 0b10001110);
	SetGate(0x08, &Exc8, 0b10001110);
	SetGate(0x0A, &ExcA, 0b10001110);
	SetGate(0x0B, &ExcB, 0b10001110);
	SetGate(0x0C, &ExcC, 0b10001110);
	SetGate(0x0D, &ExcD, 0b10001110);
	SetGate(0x0E, &ExcE, 0b10001110);
	SetGate(0x10, &Exc10, 0b10001110);
	SetGate(0x11, &Exc11, 0b10001110);
	SetGate(0x12, &Exc12, 0b10001110);
	SetGate(0x14, &Exc14, 0b10001110);
	SetGate(0x1E, &Exc1E, 0b10001110);
	
	SetGate(0x20, &SystemTimerInt, 0b10001110);
	SetGate(0x21, &KeyboardInt, 0b10001110);
	TimeSinceStart = 0;
	
	//Set IRQ 0 to trigger every 50 microseconds
	Output8(0x43, 0b00110100); //Set counter 0 to Rate Generator
	Output8(0x40, 0x0B); //Set lower byte 0x3C
	Output8(0x40, 0xE9); //set higher byte 0x00
	__asm__("MOV (Pointer), %eax; LIDT (%eax);sti");
	
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
			Paging.Pages[PML4Index] = (long)NewTable | 3; //Temporary
		}
		long* PDPTTable = (long*)((long)Paging.Pages[PML4Index] & 0x7FFFFFFFFF000);
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
		PETable[PTIndex] = (long)AlignedPhyAddr | 0x83;
	}
	Paging.Activate();
	
	//Serial.WriteString(0x1, "\r\nStarting Video setup");
	//Graphics setup
	GUI = Video((vbe_mode_info_struct*)((long)mbd->vbe_mode_info));
	unsigned long End = 0xA00000;
	GUI.SecondFrameAddress = (unsigned char*)End;
	//Map the secondary buffer
	Serial.WriteString(0x1, "\r\nSetup main graphics buffer");
	for(unsigned long i = End; i <= End + (GUI.BytesPerLine * GUI.Height); i += 0x1000)
	{
		MemorySeg* NextFrame = PhysMemory.FindFreePhyAddr();
		PhysMemory.UsePhyAddr(NextFrame, MEMORYSEG_LOCKED); 
		Paging.MapAddress(NextFrame->BaseAddress, i);
		
	}
	Serial.WriteString(0x1, "\r\nDone! ");
	
	Halt = Process((void*)&SystemIdle, PhysMemory.UseFreePhyAddr(), 50);
	Test = Process((void*)&TestProcess, PhysMemory.UseFreePhyAddr(), 100);
	CurrentProcess = &Halt;
	CurrentProcess->NextProcess = CurrentProcess;
	Test.Start();
	GraphDriver = Process((void*)&Graphics, PhysMemory.UseFreePhyAddr(), 150);
	GraphDriver.Start();
	//Enables multitasking and PIT(As well as other IRQs)
	
	Multitasking = true;
	Output8(PICM_Dat, 0xFC);
	Output8(PICS_Dat, 0xFF);
	//Serial.WriteString(0x1, "\r\nStarting multitasking");
	SystemIdle();
}
/////////////////////KERNEL END///////////////////////////