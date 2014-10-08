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
long CommandCount = 0;
multiboot_info_t* mbd;
char Command[1000];
unsigned int Value;
String Test1, Test2;
/////////////////////KERNEL START///////////////////////////
extern "C" void Kernel_Start()
{
	__asm__("PUSH %RAX; MOV $0x10, %RAX;MOV %RAX, %DS; MOV %RAX, %SS; POP %RAX");
	PrintString("Creating memory map... ", 0x0A);
	memory_map_t* mmap = (memory_map*)((long)mbd->mmap_addr);
	long MemoryAddr = 0x0;
	PhyMemMap = (MemorySeg*)&KernelEnd;
	MemorySeg* CurrentMap = PhyMemMap;
	
	while((long)mmap < (long)mbd->mmap_addr + (long)mbd->mmap_length)
	{
		long BaseAddr = (((long)mmap->base_addr_high) << 32) + (long)mmap->base_addr_low;
		long Length = (((long)mmap->length_high) << 32) + (long)mmap->length_low;
		if(MemoryAddr < BaseAddr)
		{
			MemoryAddr = BaseAddr;
			if(MemoryAddr % MemorySegSize != 0)MemoryAddr += MemorySegSize - (MemoryAddr % MemorySegSize);
		}
		while(MemoryAddr + MemorySegSize < BaseAddr + Length)
		{
			CurrentMap->BaseAddress = MemoryAddr;
			if(mmap->type == 1)
			{
				CurrentMap->Usage = 0;
			}
			else
			{
				CurrentMap->Usage = 2;
			}
			if(MemoryAddr + MemorySegSize > BaseAddr + Length && CurrentMap->Usage == 0)
			{
				memory_map_t* Pointer = (memory_map_t*)((long)mmap + mmap->size + sizeof(unsigned int));
				if(Pointer->type == 0)
				{
					CurrentMap->Usage = 2;
				}
			}
			CurrentMap += 1;
			MemoryAddr += MemorySegSize;
		}
		mmap = (memory_map_t*)((long)mmap + (long)mmap->size + (long)sizeof(unsigned int));
	}
	CurrentMap->Usage = (char)0xFF;
	PhyMemMapEnd = CurrentMap;
	MemorySeg* LoopChk = FindPhyAddr((long*)&KernelEnd);
	for(MemorySeg* KernelPointer = FindPhyAddr(0x0); KernelPointer <= LoopChk; KernelPointer += 1) //Setting Kernel memory blocks as in-use
	{
		UsePhyAddr(KernelPointer, 0x02);
	}
	LoopChk = FindPhyAddr((long*)PhyMemMapEnd);
	for(MemorySeg* Pointer = FindPhyAddr((long*)PhyMemMap); Pointer <= LoopChk; Pointer += 1)  //Setting memory map's memory blocks as in-use
	{
		UsePhyAddr(Pointer, 0x02);
	}
	
	PrintString(" [Done]\r\nSetting up stack...", 0x0A);
	LoopChk = FindPhyAddr((long*)StackBase);
	for(MemorySeg* Position = FindPhyAddr((long*)StackBottom); Position <= LoopChk; Position += 1)
	{
		UsePhyAddr(Position, 0x02); //TODO: Check if memory segments in use, perhaps make the stack dynamically located
	}
	
	
	for(char* i = StackBase; i >= StackBottom; i--)
	{
		*i = (char)0;
	}
	__asm__("MOVL StackBase, %EBP\n\t"
			"MOVL StackBase, %ESP");
			
	PrintString(" [Done]\r\nSetting up IDT...", 0x0A);
	IDTPos = (long*)PhyMemMapEnd + MemorySegSize;
	UsePhyAddr(FindPhyAddr(IDTPos), 0x02);
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
	
	for(int i = 0; i < 20; i++)
	{
		KeyQueue[i] = 0;
	}
	
	PrintString(" [Done]\r\n\r\n", 0x0A);
	
	Test1 = String("TESTDA");
	//Test2 = String("TESTDATAS");
	
	//PrintString(Test1 + Test2, 0x0A);
	PrintString("\r\nString works!", 0x0A);
	while(1);
}
/////////////////////KERNEL END///////////////////////////