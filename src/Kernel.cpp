long TimeSinceStart;

unsigned char Hex[17] = "0123456789ABCDEF";
#include "IO.cpp"
#include "Serial.cpp"
#include "E820.h"
#include "Memory/MemoryMap.cpp"
#include "Memory/Paging.cpp"
#include "Memory/Malloc.cpp"
#include "BasicFunctions.cpp"

#include "Keyboard.cpp"


#include "Video/Video.cpp"

#include "Miscellaneous/Miscellaneous.cpp"

#include "Interrupts/Exceptions.cpp"
#include "Interrupts/IDT.cpp"

int Header[12] __attribute__((section (".Multiboot"))) = 
{
	0x1BADB002, 0x00000004, -(0x1BADB002 + 0x00000004),
	0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000,
	1024, 768, 24
};


short* PML4 = (short*)&PML4Temp;
char* StackBase = (char*)0x001F000;
char* StackBottom = (char*)0x001D000;
char extern KernelStart;
char extern KernelEnd;
long* IDTPos;
multiboot_info_t* mbd;
MemoryMap PhysMemory;
PageFile Paging;
//SerialController Serial;
unsigned short MemoryModel;
long TestVar;
char* BDA = (char*)0x400;
/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX"); 
	
	//Setup Serial ports
	Serial.Setup(BDA);
	Serial.WriteString(0x1, "Testing Serial Port!");
	
	//Setting up memory map
	PhysMemory.Initialise(mbd, (MemorySeg*)&KernelEnd, 0x1000);
	MemorySeg* LoopChk = PhysMemory.FindPhyAddr(&KernelEnd);
	MemorySeg* KernelPointer;
	
	Serial.WriteString(0x1, "\r\nSetting Kernel memory blocks");
	for(KernelPointer = PhysMemory.FindPhyAddr(0x0); KernelPointer <= LoopChk; KernelPointer++) //Setting Kernel memory blocks as in-use
	{
		PhysMemory.UsePhyAddr(KernelPointer, MEMORYSEG_LOCKED);
	}
	
	Serial.WriteString(0x1, "\r\nSet up single IDT memory block");
	//Setting up IDT
	IDTPos = (long*)PhysMemory[PhysMemory.Size] + PhysMemory.MemorySegSize;
	PhysMemory.UsePhyAddr(PhysMemory.FindPhyAddr(IDTPos), MEMORYSEG_LOCKED);
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
	
	//Set IRQ 0 to trigger every 50ms
	Output8(0x43, 0b00110100); //Set counter 0 to Rate Generator
	Output8(0x40, 0x0B); //Set lower byte
	Output8(0x40, 0xE9); //set higher byte 
	__asm__("MOV (Pointer), %eax; LIDT (%eax);sti");
	Output8(PICM_Dat, 0xFC);
	Output8(PICS_Dat, 0xFF);
	
	Paging.Initialise();
	// 1:1 mapping for the first 10MB of memory
	Serial.WriteString(0x1, "\r\nMap all remaining memory blocks in first 10MB\r\n");
	for(MemorySeg* Position = PhysMemory.FindPhyAddr((void*)0x0); Position <= PhysMemory.FindPhyAddr((void*)0xA00000); Position++)
	{
		PhysMemory.UsePhyAddr(Position, MEMORYSEG_LOCKED); 
	}
	
	for(unsigned long i = 0; i < 0xA00000; i += 0x1000)
	{
		Paging.MapAddress(i, i);
	}
	
	Serial.WriteString(0x1, "\r\nStarting Video setup");
	
	//Graphics setup
	GUI = Video((vbe_mode_info_struct*)((long)mbd->vbe_mode_info));
		
	//Mapping Video memory starting at 0xA00000
	unsigned long End = 0xA00000;
	Serial.WriteString(0x1, "\r\nMap main video memory");
	for(unsigned long i = 0; i < GUI.BytesPerLine * GUI.Height; i += 0x1000)
	{
		Paging.MapAddress(((unsigned long)GUI.FrameAddress + i), 0xA00000 + i);
		End = 0xA00000 + i;
	}
	GUI.FrameAddress = (char*)0xA00000;
	End += 0x1000;
	GUI.SecondFrameAddress = (char*)End;
	//Map the secondary buffer
	Serial.WriteString(0x1, "\r\nMap second video memory");
	for(unsigned long i = End; i <= End + (GUI.BytesPerLine * GUI.Height); i += 0x1000)
	{
		MemorySeg* NextFrame = PhysMemory.FindFreePhyAddr();
		PhysMemory.UsePhyAddr(NextFrame, MEMORYSEG_LOCKED); 
		Paging.MapAddress(NextFrame->BaseAddress, i);
		
	}
	Serial.WriteString(0x1, "\r\nMapping done!\r\nStarting...");
	Paging.Activate();
	char shade = 128;
	long Time = 0;
	int x = 214, y = 532;
	int xVel = 15, yVel = 18;
	while(1)
	{
		DrawString("Test string", 11, x, y);
		GUI.Update();
		x += xVel;
		y += yVel;
		if(x < 0)
		{
			x = 0;
			xVel = -xVel;
		}
		else if(x + 99 > GUI.Width)
		{
			x = GUI.Width - 99;
			xVel = -xVel;
		}
		if(y < 0)
		{
			y = 0;
			yVel = -yVel;
		}
		else if(y + 9 > GUI.Height)
		{
			y = GUI.Height - 9;
			yVel = -yVel;
		}
		Time = TimeSinceStart;
	}
}
/////////////////////KERNEL END///////////////////////////
