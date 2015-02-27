long TimeSinceStart;

#include "Miscellaneous/String.h"

#include "Keyboard.cpp"
#include "BasicFunctions.cpp"
#include "VGAText/VGA.cpp"
#include "E820.h"
#include "IO.cpp"

#include "Miscellaneous/Miscellaneous.cpp"

#include "Memory/MemoryMap.cpp"
#include "Memory/Paging.cpp"
#include "Memory/Malloc.cpp"

#include "Interrupts/Exceptions.cpp"
#include "Interrupts/IDT.cpp"


#include "Miscellaneous/String.cpp"

int Header[12] __attribute__((section (".Multiboot"))) = 
{
	0x1BADB002, 0x00000000, -(0x1BADB002 + 0x00000000),
	0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000
};

short* volatile PML4 = (short*)&PML4Temp;
char* volatile StackBase = (char*)0x001FFFFF;
char* volatile StackBottom = (char*)0x001FE000;
long extern KernelStart;
long extern KernelEnd;
long* IDTPos;
multiboot_info_t* mbd;
MemoryMap PhysMemory;
/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX"); 
	
	//Setting up memory map
	PrintString("Creating memory map...", 0x0A);
	PhysMemory.Initialise(mbd, (MemorySeg*)&KernelEnd, 0x1000);
	MemorySeg* LoopChk = PhysMemory.FindPhyAddr(&KernelEnd);
	for(MemorySeg* KernelPointer = PhysMemory.FindPhyAddr(0x0); KernelPointer <= LoopChk; KernelPointer++) //Setting Kernel memory blocks as in-use
	{
		PhysMemory.UsePhyAddr(KernelPointer, 0x02);
	}
	
	//Setting up Stack
	PrintString(" [Done]\r\nSetting up stack...", 0x0A);
	LoopChk = PhysMemory.FindPhyAddr((long*)StackBase);
	for(MemorySeg* Position = PhysMemory.FindPhyAddr((long*)StackBottom); Position <= LoopChk; Position += 1)
	{
		PhysMemory.UsePhyAddr(Position, 0x02); //TODO: Check if memory segments in use, perhaps make the stack dynamically located
	}
	for(char* i = StackBase; i >= StackBottom; i--)
	{
		*i = (char)0;
	}
	__asm__("MOVL StackBase, %EBP\n\t"
			"MOVL StackBase, %ESP");
	
	//Setting up IDT
	PrintString(" [Done]\r\nSetting up IDT...", 0x0A);
	IDTPos = (long*)PhysMemory[PhysMemory.Size] + PhysMemory.MemorySegSize;
	PhysMemory.UsePhyAddr(PhysMemory.FindPhyAddr(IDTPos), 0x02);
	IDT = (IDTStruct*)IDTPos;
	for(char* i = (char*)IDT; (long)i < (long)IDT + sizeof(IDTStruct); i++)
	{
		*i = 0;
	}
	IDT->Pointer.Limit = 0xFF * sizeof(IDTDescr) - 1;
	IDT->Pointer.Base = (long*)IDT;
	Pointer = (long*)&(IDT->Pointer);
	PICMapping_Init(0x20, 0x28); //Initialise the PICs, Master mapped to 0x21 - 0x27, Slave mapped to 0x28 - 0x30
	
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
	PrintString(" [Done]\r\nSetting up keyboard...", 0x0A);
	Output8(PICM_Dat, 0xFC);
	Output8(PICS_Dat, 0xFF);
	
	PrintString(" [Done]\r\n\r\n", 0x0A);
	
	while(1);
}
/////////////////////KERNEL END///////////////////////////