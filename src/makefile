all:
	clear
	clear
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-g++ -o /dat/OSDev/tmp/Kernel.o -c /dat/OSDev/src/Kernel.cpp -ffreestanding --no-exceptions -fpermissive -std=c++11 -m64 -mno-red-zone -O2
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Stage2.o -c /dat/OSDev/src/Stage2.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Interrupts.o -c /dat/OSDev/src/Interrupts/Interrupts.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-ld -o /dat/OSDev/tmp/Kernel.elf /dat/OSDev/tmp/Stage2.o /dat/OSDev/tmp/Interrupts.o /dat/OSDev/tmp/Kernel.o -T /dat/OSDev/src/Linker.lin
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-objcopy -O elf32-i386 /dat/OSDev/tmp/Kernel.elf /dat/OSDev/MountPoint/Kernel.elf

asm:
	clear
	clear
	/dat/OSDev/Cross-Compiler/x86_64-elf/bin/objdump -d /dat/OSDev/MountPoint/Kernel.elf -m i386:x86-64
	
headers:
	clear
	clear
	/dat/OSDev/Cross-Compiler/x86_64-elf/bin/objdump -x /dat/OSDev/MountPoint/Kernel.elf
	
debug: 
	clear
	clear
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-gcc -o /dat/OSDev/tmp/Kernel.o -c /dat/OSDev/src/Kernel.cpp -ffreestanding --no-exceptions -fpermissive -std=c++11 -m64 -mno-red-zone
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Stage2.o -c /dat/OSDev/src/Stage2.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/Interrupts.o -c /dat/OSDev/src/Interrupts/Interrupts.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-as -o /dat/OSDev/tmp/ProcessSwitch.o -c /dat/OSDev/src/ProcessSwitch.s
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-ld -o /dat/OSDev/tmp/Kernel.elf /dat/OSDev/tmp/Stage2.o /dat/OSDev/tmp/Interrupts.o /dat/OSDev/tmp/ProcessSwitch.o /dat/OSDev/tmp/Kernel.o -T /dat/OSDev/src/Linker.lin
	/dat/OSDev/Cross-Compiler/bin/x86_64-elf-objcopy -O elf32-i386 /dat/OSDev/tmp/Kernel.elf /dat/OSDev/MountPoint/Kernel.elf

mount:
	sudo mount /dat/OSDev/VM/OSHDD.vhd /dat/OSDev/MountPoint