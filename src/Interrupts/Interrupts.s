.code64

RAXTEMP: .quad 0x0

.global KeyboardInt
.global SystemTimerInt
.global ProcessSwitchInt
.global Exc0, Exc1, Exc2, Exc3, Exc4, Exc5
.global Exc6, Exc7, Exc8, ExcA, ExcB, ExcC
.global ExcD, ExcE, Exc10, Exc11, Exc12, Exc13
.global Exc14, Exc1E
.global SysCall, SwitchThread
	
KeyboardInt:
	CALL KeyboardInterrupt
	IRETQ
	
SystemTimerInt:
	CALL SystemTimerInterrupt
	IRETQ

################ Exception handlers ######################
Exc0:
	CALL DivideByZeroExc
	IRETQ
	
Exc1:
	POP %RAX
	CALL DebugExc
	IRETQ

Exc2:
	CALL NonMaskableExc
	IRETQ

Exc3:
	CALL BreakPointExc
	IRETQ

Exc4:
	CALL OverflowExc
	IRETQ

Exc5:
	CALL BoundRangeExc
	IRETQ

Exc6:
	CALL InvalidOpExc
	IRETQ

Exc7:
	CALL DeviceNotAvailExc
	IRETQ

Exc8:
	CALL DoubleFaultExc
	IRETQ

ExcA:
	CALL InvalidTSSExc
	IRETQ

ExcB:
	CALL SegNotPresentExc
	IRETQ

ExcC:
	CALL StackSegExc
	IRETQ

ExcD:
	MOV RAXTEMP, %RAX
	POP %RAX
	CALL GenProtExc
	MOV %RAX, RAXTEMP
	IRETQ
	
ExcE:
	MOV RAXTEMP, %RAX
	POP %RAX
	CALL PageFaultExc
	MOV %RAX, RAXTEMP
	IRETQ

Exc10:
	CALL x87FloatPExc
	IRETQ

Exc11:
	CALL AlignChkExc
	IRETQ

Exc12:
	CALL MachineChkExc
	IRETQ

Exc13:
	CALL SIMDFloatPExc
	IRETQ

Exc14:
	CALL VirtualExc
	IRETQ

Exc1E:
	CALL SecurityExc
	IRETQ
	
SysCall:
	CALL SysCallSwitch
	IRETQ

SwitchThread:
	CALL SwitchProcesses
	IRETQ
	