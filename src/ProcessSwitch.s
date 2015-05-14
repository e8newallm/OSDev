.code64
.section .text
	
.global ProcessSwitch, RotateProcesses

#RDI = (CurrentProcess->RSP)'s address
#RSI = NextProcess->RSP
#RDX = New CR3

ProcessSwitch:
	PUSH %RBP
	PUSH %RBX
	PUSH %R12
	PUSH %R13
	PUSH %R14
	PUSH %R15
	
	MOV %RSP, (%RDI)
	MOV %RDX, %CR3
	MOV %RSI, %RSP
	
	POP %R15
	POP %R14
	POP %R13
	POP %R12
	POP %RBX
	POP %RBP
	
	RETQ
