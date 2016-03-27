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
	PushAll();
	//CLI();
	unsigned char PressedKey = Input8(0x60);
	if(PressedKey < 0x58)
	{
		PutKeyPress(ScanCodes1[PressedKey]);
	}
	PICEndInt((char)1);
	//STI();
	PopAll();
}

extern "C" void SystemTimerInterrupt()
{
	TimeSinceStart += 10;
	CurrentThreadDuration += 10;
	CurrentPeriodDuration += 10;
	Output8(0x40, 0x9B);
	Output8(0x40, 0x2E);
	PICEndInt((char)0);
	if(!CurrentThreadPeriod && CurrentPeriodDuration >= NormalThreadPeriod)
	{
		Serial.WriteString(0x1, "\r\n\r\nMoving to fast threads");
		RoundRobinThread = CurrentThread;
		CurrentThreadPeriod = !CurrentThreadPeriod;
		CurrentPeriodDuration = 0;
		SwitchProcesses();
	}
	else if(CurrentThreadPeriod && CurrentPeriodDuration >= QuickThreadPeriod)
	{
		Serial.WriteString(0x1, "\r\n\r\nMoving to normal threads");
		CurrentThreadPeriod = !CurrentThreadPeriod;
		CurrentPeriodDuration = 0;
		SwitchProcesses();
	}
	else if(CurrentThreadDuration >= CurrentThread->Duration)
	{
		Serial.WriteString(0x1, "\r\n\r\nPreemptive swap");
		SwitchProcesses();
	}
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
		case PROCESS_MAKE_ID: //Create Process %RAX = PROCESS_MAKE_ID; %RBX = Main; %RCX = Name; Returns ID = %RAX
		{
			int ID = Process_Make((void*)RBX, (const char*)RCX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		case THREAD_MAKE_ID: //Create Thread %RAX = THREAD_MAKE_ID; %RBX = Main; %RCX = ProcessID; Returns ID = %RAX
		{
			int ID = GetProcess(RCX)->Thread_Create((void*)RBX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		
		case QTHREAD_MAKE_ID: //Create Thread %RAX = QTHREAD_MAKE_ID; %RBX = Main; %RCX = ProcessID; %RDX = Duration; Returns ID = %RAX
		{
			int ID = GetProcess(RCX)->QThread_Create((void*)RBX, RDX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		
		case THREAD_MAKE_ID_SELF: //Create Thread %RAX = THREAD_MAKE_ID; %RBX = Main; Returns ID = %RAX
		{
			int ID = CurrentThread->OwnerProcess->Thread_Create((void*)RBX);
			asm volatile("MOV %0, %%RAX"::"m"(ID) : "%rax");
			break;
		}
		
		case QTHREAD_MAKE_ID_SELF: //Create Thread %RAX = QTHREAD_MAKE_ID; %RBX = Main; %RCX = Duration; Returns ID = %RAX
		{
			int ID = CurrentThread->OwnerProcess->QThread_Create((void*)RBX, RCX);
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
			Serial.WriteString(0x1, "\r\nMapping vid Memory: ");
			Serial.WriteLongHex(0x1, (GUI.BytesPerLine * GUI.Height));
			for(unsigned long i = 0; i <= (GUI.BytesPerLine * GUI.Height); i += 0x1000)
			{
				CurrentThread->Page->MapAddress(((unsigned long)GUI.FrameAddress + i), (long)RBX + i);
			}
			break;
		}
	}
}
