.code32

.global BootStart

.data
	GDT:
    .quad 0x0000000000000000             #Null Descriptor - should be present.
    .quad 0x0020980000000000             #64-bit code descriptor. 
    .quad 0x0000920000000000             #64-bit data descriptor. 
	GDTPointer:
    .word . - GDT - 1                    #16-bit Size (Limit) of GDT.
	.quad GDT
	
.section .text

BootStart:
	CLI
	MOV %EBX, mbd
	
	#Make sure Paging is disabled
	MOV %CR0, %EAX
	AND $0b01111111111111111111111111111111, %EAX
	MOV %EAX, %CR0
	
	#Check CR3 bits [0-11] are 000h
	MOV %CR3, %EAX
	AND $0x00, %EAX
	MOV %EAX, %CR3 
	
	#Enable Physical Address Extension
	MOV %CR4, %EAX
	OR $0x00020, %EAX
	MOV %EAX, %CR4
	
	#Load CR3 with Physical base address of Level 4 page map table (PML4)
	LEA (PML4), %EAX
	MOV %EAX, %CR3
	
	#Enable IA-32e
	MOV $0xC0000080, %ECX 
	RDMSR
	OR $0b100000000, %EAX
	WRMSR
	
	#Re-enable Paging
	MOV %CR0, %EAX
	OR $0b10000000000000000000000000000001, %EAX
	MOV %EAX, %CR0
	
	MOVL StackBase, %EBP
	MOVL StackBase, %ESP

	
	LGDT (GDTPointer)
	LJMP $0x0008, $Kernel_Start
	
	#Go back to real mode
	MOV %CR0, %EAX
	AND $0xFFFFFFFE, %EAX
	MOV %EAX, %CR0
	
	#Change video mode
	MOV $0x00, %AH
	MOV $0x12, %AL
	INT $0x10
	
	#Go back to protected mode
	MOV %CR0, %EAX
	OR 0x1, %EAX
	MOV %EAX, %CR0
	