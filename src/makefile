all:
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-gcc -o /dat/OSDev/tmp/Kernel.o -c /dat/OSDev/src/Kernel.cpp -ffreestanding --no-exceptions -fpermissive -std=c++11 -Wreturn-local-addr -m64 -mno-red-zone
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Stage2.o -c /dat/OSDev/src/Stage2.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Interrupts.o -c /dat/OSDev/src/Interrupts/Interrupts.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-ld -o /dat/OSDev/tmp/Kernel.elf /dat/OSDev/tmp/Stage2.o /dat/OSDev/tmp/Interrupts.o /dat/OSDev/tmp/Kernel.o -T /dat/OSDev/src/Linker.lin
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-objcopy -O elf32-i386 /dat/OSDev/tmp/Kernel.elf /dat/OSDev/MountPoint/Kernel.elf