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
