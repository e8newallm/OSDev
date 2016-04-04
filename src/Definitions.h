//SYSCALL FUNCTION IDS

#define PROCESS_MAKE_ID 0
#define THREAD_MAKE_ID 1
#define QTHREAD_MAKE_ID 2
#define THREAD_MAKE_ID_SELF 3
#define QTHREAD_MAKE_ID_SELF 4
#define PAGE_MAP_ID 5
#define MAP_VIDEO_MEM_ID 6
#define UPDATE_WINDOW_ID 7
#define GET_MILLI_ID 8

#define BACK_PROCESS_MAKE_ID 9

#define CLI() __asm__("CLI");
#define STI() __asm__("STI");

unsigned char Hex[17] = "0123456789ABCDEF";
unsigned char Dec[11] = "0123456789";

//Process definitions

#define StackSize 0x100000
#define TSSOffset 0x80000
#define StackSpaceStart 0xFF0000000
#define StackSpaceEnd StackSpaceStart + (StackSize * 255)

//Malloc definitions

#define MBlockHeader_Free 0
#define MBlockHeader_InUse 1
#define MBlockHeader_Start (unsigned char)254
#define MBlockHeader_End (unsigned char)255

