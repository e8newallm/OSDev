void PICEndInt(char IRQ)
{
	if(IRQ >= 8)
	{
		Output8(PICS_Com, PIC_EOI);
	}
	Output8(PICM_Com, PIC_EOI);
}

void SetGate(unsigned char GateAddr, long* Offset, unsigned char Type)
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
	CLI();
	unsigned char PressedKey = Input8(0x60);
	if(PressedKey < sizeof(ScanCodes1))
	{
		PutKeyPress(ScanCodes1[PressedKey]);
	}
	PICEndInt((char)1);
	STI();
}

extern "C" void SystemTimerInterrupt()
{
	TimeSinceStart += 10;
	CurrentThreadDuration += 10;
	Output8(0x40, 0x9B);
	Output8(0x40, 0x2E);
	PICEndInt((char)0);
	Scheduling();
}

extern "C" void SysCall();
extern "C" void SwitchThread();

extern "C" void SysCallSwitch()
{
	long RAX, RBX, RCX, RDX, R8; //RAX = FUNCTION
	asm volatile("MOV %%RAX, %0":"=m"(RAX) :: "%rax");
	asm volatile("MOV %%RBX, %0":"=m"(RBX) :: "%rbx");
	asm volatile("MOV %%RCX, %0":"=m"(RCX) :: "%rcx");
	asm volatile("MOV %%RDX, %0":"=m"(RDX) :: "%rdx");
	
	asm volatile("MOV %%R8, %0":"=m"(R8) :: "%r8");
	
	switch(RAX)
	{
		case PROCESS_MAKE_ID: //Create Process %RAX = PROCESS_MAKE_ID; %RBX = Main; %RCX = Name; %RDX = Priority; Returns ID = %RAX
		{
			int ID = Process_Make((void*)RBX, (const char*)RCX, (int)RDX, false);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		
		case THREAD_MAKE_ID: //Create Thread %RAX = THREAD_MAKE_ID; %RBX = Main; %RCX = ProcessID; %RDX = Priority; Returns ID = %RAX
		{
			int ID = GetProcess(RCX)->Thread_Create((void*)RBX, RDX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		case THREAD_MAKE_ID_SELF: //Create Thread %RAX = THREAD_MAKE_ID; %RBX = Main; %RCX = Priority; Returns ID = %RAX
		{
			int ID = CurrentThread->OwnerProcess->Thread_Create((void*)RBX, RCX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}

		case PAGE_MAP_ID: //Map virtual address %RAX = PAGE_MAP_ID; %RBX = Size; Returns nothing
		{
			AddVMemory(RBX);
			break;
		}
		
		case MAP_VIDEO_MEM_ID: //Map virtual address %RAX = MAP_VIDEO_MEM_ID; %RBX = Starting address; Returns nothing
		{
			for(unsigned long i = 0; i <= (GUI.BytesPerLine * GUI.Height); i += 0x1000)
			{
				CurrentThread->Page->MapAddress(((unsigned long)GUI.FrameAddress + i), (long)RBX + i);
			}
			break;
		}
		
		case GET_MILLI_ID: //Returns count of milliseconds since system started %RAX = GET_MILLI_ID; Return %RAX = Millis
		{
			asm volatile("MOV %0, %%RAX"::"m"(TimeSinceStart) : "%rax");
			break;
		}
		
		case THREAD_START_ID: //Starts a thread %RAX = THREAD_START_ID; %RBX = ThreadID; Return nothing
		{
			CurrentThread->OwnerProcess->GetThread(RBX)->Start();
			break;
		}
		
		case WAIT_THREAD_ID: //Waits for thread to end %RAX = WAIT_THREAD_ID; %RBX = ProcessID; %RCX = ThreadID; Return nothing
		{
			Thread* WaitingThread = GetProcess(RBX)->GetThread(RCX);
			CurrentThread->WaitingEndQueueNext = WaitingThread->WaitingEndQueue;
			WaitingThread->WaitingEndQueue = CurrentThread;
			CurrentThread->Block();
			break;
		}
	}
}
