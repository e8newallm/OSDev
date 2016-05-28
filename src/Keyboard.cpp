#define Key_Escape 0x7F
#define Key_Enter 0x80
#define Key_LCtrl 0x81
#define Key_RCtrl 0x82
#define Key_LShift 0x83
#define Key_RShift 0x84
#define Key_LAlt 0x85
#define Key_RAlt 0x86
#define Key_CapLock 0x87
#define Key_NumLock 0x88

#define Key_F1 0x90
#define Key_F2 0x91
#define Key_F3 0x92
#define Key_F4 0x93
#define Key_F5 0x94
#define Key_F6 0x95
#define Key_F7 0x96
#define Key_F8 0x97
#define Key_F9 0x98
#define Key_F10 0x99
#define Key_F11 0x9A
#define Key_F12 0x9B

#define Key_Backspace '\b'
#define Key_Tab '\t'

unsigned char KeyQueue[20];

#define IsCharacter(x) (((unsigned char)x < Key_Escape || (unsigned char)x > Key_NumLock) && ((unsigned char)x < Key_F1 || (unsigned char)x > Key_F12) && (unsigned char)x != 0) 
unsigned char GetKeyPress()
{
	if(KeyQueue[0] == 0)
	{
		return 0; //Key Queue is empty
	}
	else
	{
		char Key = KeyQueue[0];
		for(int i = 0; i < 19; i++)
		{
			KeyQueue[i] = KeyQueue[i+1];
		}
		KeyQueue[19] = 0;
		return Key;
	}
}

bool PutKeyPress(char Key)
{
	for(int i = 0; i < 20; i++)
	{
		if(KeyQueue[i] == 0)
		{
			KeyQueue[i] = Key;
			return true;
		}
	}
	return false;
}

unsigned char ScanCodes1[] = 
{
 '\0', Key_Escape, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 
 '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', Key_Enter, 
 Key_LCtrl, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
 Key_LShift, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', Key_RShift, '*', 
 Key_LAlt, ' ', Key_CapLock, Key_F1, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6, Key_F7, Key_F8, Key_F9, Key_F10,
 Key_NumLock, '7', '8', '9', '4', '5', '6', '1', '2', '3', '0', '.', 0, 0, 0, Key_F11, Key_F12,
};
