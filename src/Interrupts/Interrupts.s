.code64

.global KeyboardInt
.global SystemTimerInt
.global Exc0, Exc1, Exc2, Exc3, Exc4, Exc5
.global Exc6, Exc7, Exc8, ExcA, ExcB, ExcC
.global ExcD, ExcE, Exc10, Exc11, Exc12, Exc13
.global Exc14, Exc1E

.macro PushAll
	PUSH %RAX
	PUSH %RBX
	PUSH %RCX
	PUSH %RDX
	PUSH %RBP
	PUSH %RDI
	PUSH %RSI
.endm

.macro PopAll
	POP %RSI
	POP %RDI
	POP %RBP
	POP %RDX
	POP %RCX
	POP %RBX
	POP %RAX
.endm

KeyboardInt:
	PushAll
	CALL KeyboardInterrupt
	PopAll
	IRETQ
	
SystemTimerInt:
	PushAll
	CALL SystemTimerInterrupt
	PopAll
	IRETQ

################ Exception handlers ######################
	
Exc0:
	PushAll
	CALL DivideByZeroExc
	PopAll
	IRETQ
	
Exc1:
	PushAll
	CALL DebugExc
	PopAll
	IRETQ

Exc2:
	PushAll
	CALL NonMaskableExc
	PopAll
	IRETQ

Exc3:
	PushAll
	CALL BreakPointExc
	PopAll
	IRETQ

Exc4:
	PushAll
	CALL OverflowExc
	PopAll
	IRETQ

Exc5:
	PushAll
	CALL BoundRangeExc
	PopAll
	IRETQ

Exc6:
	PushAll
	CALL InvalidOpExc
	PopAll
	IRETQ

Exc7:
	PushAll
	CALL DeviceNotAvailExc
	PopAll
	IRETQ

Exc8:
	PushAll
	CALL DoubleFaultExc
	PopAll
	IRETQ

ExcA:
	PushAll
	CALL InvalidTSSExc
	PopAll
	IRETQ

ExcB:
	PushAll
	CALL SegNotPresentExc
	PopAll
	IRETQ

ExcC:
	PushAll
	CALL StackSegExc
	PopAll
	IRETQ

ExcD:
	PushAll
	CALL GenProtExc
	PopAll
	IRETQ

ExcE:
	POP %RAX
	CALL PageFaultExc
	IRETQ

Exc10:
	PushAll
	CALL x87FloatPExc
	PopAll
	IRETQ

Exc11:
	PushAll
	CALL AlignChkExc
	PopAll
	IRETQ

Exc12:
	PushAll
	CALL MachineChkExc
	PopAll
	IRETQ

Exc13:
	PushAll
	CALL SIMDFloatPExc
	PopAll
	IRETQ

Exc14:
	PushAll
	CALL VirtualExc
	PopAll
	IRETQ

Exc1E:
	PushAll
	CALL SecurityExc
	PopAll
	IRETQ

