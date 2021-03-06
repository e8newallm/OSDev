extern "C" void DivideByZeroExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Divide-By-Zero error (0x0)");
}
extern "C" void DebugExc(long Address)
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Serial.WriteString(0x1, "\r\n\r\nAddress: ");
	Serial.WriteLongHex(0x1, Address);
	Kernel_Panic("Debug error (0x1)");
}

extern "C" void NonMaskableExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Non-Maskable Interrupt error (0x2)");
}

extern "C" void BreakPointExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Breakpoint error (0x3)");
}

extern "C" void OverflowExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Overflow error (0x4)");
}

extern "C" void BoundRangeExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Bound Range Exceeded error (0x5)");
}

extern "C" void InvalidOpExc(long Address)
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Serial.WriteString(0x1, "\r\n\r\nAddress: ");
	Serial.WriteLongHex(0x1, Address);
	Kernel_Panic("Invalid Opcode error (0x6)");
}

extern "C" void DeviceNotAvailExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Device not available error (0x7)");
}

extern "C" void DoubleFaultExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Double fault error (0x8)");
}

extern "C" void InvalidTSSExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Invalid TSS error (0xA)");
}

extern "C" void SegNotPresentExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Segment not present error (0xB)");
}

extern "C" void StackSegExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Stack segment fault error (0xC)");
}

extern "C" void GenProtExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	long ErrorCode;
	long VirtualAddr;
	asm volatile("MOV %%RAX, %0"
			: "=r"(ErrorCode));
	asm volatile("MOV %%CR2 , %0"
		: "=r"(VirtualAddr));
	Serial.WriteString(0x1, "\r\nError code: ");
	Serial.WriteLongHex(0x1, (long)ErrorCode);
	Serial.WriteString(0x1, " Virtual address: ");
	Serial.WriteLongHex(0x1, VirtualAddr);
	Kernel_Panic("General protection error (0xD)");
}

extern "C" void PageFaultExc()
{
	char ErrorCode;
	long VirtualAddr;
	asm volatile("MOV %%AL, %0"
			: "=r"(ErrorCode));
	asm volatile("MOV %%CR2 , %0"
		: "=r"(VirtualAddr));
	if(VirtualAddr >= StackSpaceStart && VirtualAddr < StackSpaceEnd)
	{
		long Temp = (long)PhysMemory.UseFreePhyAddr();
		Serial.WriteString(0x1, "\r\nStack Page Fault\tMapping ");
		Serial.WriteLongHex(0x1, Temp);
		Serial.WriteString(0x1, " to ");
		Serial.WriteLongHex(0x1, VirtualAddr);
		Serial.WriteString(0x1, " for ");
		Serial.WriteString(0x1, (CurrentThread->OwnerProcess)->ProcessName);
		CurrentThread->Page->MapAddress((unsigned long)Temp, (unsigned long)(VirtualAddr));
	}
	else
	{
		CLI();
		Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
		Output8(0xA1, 0xFF);
		Serial.WriteString(0x1, "\r\n\r\nError code: ");
		Serial.WriteLongHex(0x1, (long)ErrorCode);
		Serial.WriteString(0x1, " Virtual address: ");
		Serial.WriteLongHex(0x1, VirtualAddr);
		Kernel_Panic("Page fault error (0xE)");
	}
}

extern "C" void x87FloatPExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("x87 floating point error (0x10)");
}

extern "C" void AlignChkExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Alignment check error (0x11)");
}

extern "C" void MachineChkExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Machine check error (0x12)");
}

extern "C" void SIMDFloatPExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("SIMD floating point error (0x13)");
}

extern "C" void VirtualExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Virtualization error (0x14)");
}

extern "C" void SecurityExc()
{
	CLI();
	Output8(0x21, 0xFF); //Masking the PIC Master/Slave to stop all IRQs
	Output8(0xA1, 0xFF);
	Kernel_Panic("Security error (0x1E)");
}
