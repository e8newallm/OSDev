#define PICM_Com 0x20
#define PICM_Dat 0x21
#define PICS_Com 0xA0
#define PICS_Dat 0xA1

#define PIC_EOI 0x20

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

long extern KeyboardInt, SystemTimerInt;
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
	char PressedKey = Input8(0x60);
	bool Test;
	if(PressedKey < 0x58)
	{
		Test = PutKeyPress(ScanCodes1[PressedKey]);
	}
	if(!Test)
	{
		Kernel_Panic("\r\nQueue's full yo!");
	}
	PICEndInt((char)1);
}

extern "C" void SystemTimerInterrupt()
{
	TimeSinceStart++;
	PICEndInt((char)0);
}