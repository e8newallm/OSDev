.code64
.section .text
	
.global ProcessSwitch

#RDI = (CurrentProcess->RSP)'s address
#RSI = NextProcess->RSP
#RDX = New CR3

ProcessSwitch:
	PUSH %RAX
	PUSH %RCX
	PUSH %RBP
	PUSH %RBX
	PUSH %RDX
	PUSH %RSI
	PUSH %RDI
	PUSH %R8
	PUSH %R9
	PUSH %R10
	PUSH %R11
	PUSH %R12
	PUSH %R13
	PUSH %R14
	PUSH %R15
	PUSHF
	
	MOV %RSP, (%RDI)
	MOV %RDX, %CR3
	MOV %RSI, %RSP
	
	POPF
	POP %R15
	POP %R14
	POP %R13
	POP %R12
	POP %R11
	POP %R10
	POP %R9
	POP %R8
	POP %RDI
	POP %RSI
	POP %RDX
	POP %RBX
	POP %RBP
	POP %RCX
	POP %RAX
	RETQ
