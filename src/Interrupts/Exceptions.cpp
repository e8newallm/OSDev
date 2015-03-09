extern "C" void DivideByZeroExc()
{
	Kernel_Panic("Divide-By-Zero error (0x0)");
}
extern "C" void DebugExc()
{
	Kernel_Panic("Debug error (0x1)");
}

extern "C" void NonMaskableExc()
{
	Kernel_Panic("Non-Maskable Interrupt error (0x2)");
}

extern "C" void BreakPointExc()
{
	Kernel_Panic("Breakpoint error (0x3)");
}

extern "C" void OverflowExc()
{
	Kernel_Panic("Overflow error (0x4)");
}

extern "C" void BoundRangeExc()
{
	Kernel_Panic("Bound Range Exceeded error (0x5)");
}

extern "C" void InvalidOpExc()
{
	Kernel_Panic("Invalid Opcode error (0x6)");
}

extern "C" void DeviceNotAvailExc()
{
	Kernel_Panic("Device not available error (0x7)");
}

extern "C" void DoubleFaultExc()
{
	Kernel_Panic("Double fault error (0x8)");
}

extern "C" void InvalidTSSExc()
{
	Kernel_Panic("Invalid TSS error (0xA)");
}

extern "C" void SegNotPresentExc()
{
	Kernel_Panic("Segment not present error (0xB)");
}

extern "C" void StackSegExc()
{
	Kernel_Panic("Stack segment fault error (0xC)");
}

extern "C" void GenProtExc()
{
	Kernel_Panic("General protection error (0xD)");
}

extern "C" void PageFaultExc()
{
	int ErrorCode = 0;
	asm volatile("POPQ %%RAX;"
				 "MOV %%EAX, %0;"
				 :"=r" (ErrorCode)
				 :
				 :"eax");
	PrintString("\r\nError with Paging: ", 0x0A);
	//PrintString(LongToStringHex(ErrorCode), 0x0A);
	Kernel_Panic("Page fault error (0xE)");
}

extern "C" void x87FloatPExc()
{
	Kernel_Panic("x87 floating point error (0x10)");
}

extern "C" void AlignChkExc()
{
	Kernel_Panic("Alignment check error (0x11)");
}

extern "C" void MachineChkExc()
{
	Kernel_Panic("Machine check error (0x12)");
}

extern "C" void SIMDFloatPExc()
{
	Kernel_Panic("SIMD floating point error (0x13)");
}

extern "C" void VirtualExc()
{
	Kernel_Panic("Virtualization error (0x14)");
}

extern "C" void SecurityExc()
{
	Kernel_Panic("Security error (0x1E)");
}