all:
	clear
	clear
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-g++ -o /home/e8newallm/OSDev/tmp/Kernel.o -c /home/e8newallm/OSDev/src/Kernel.cpp -ffreestanding --no-exceptions -fpermissive -std=c++14 -m64 -mno-red-zone -O2 -Wall -Wextra -pedantic
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Stage2.o -c /home/e8newallm/OSDev/src/Stage2.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Interrupts.o -c /home/e8newallm/OSDev/src/Interrupts/Interrupts.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Switch.o -c /home/e8newallm/OSDev/src/Switch.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-ld -o /home/e8newallm/OSDev/tmp/Kernel.elf /home/e8newallm/OSDev/tmp/Stage2.o /home/e8newallm/OSDev/tmp/Switch.o /home/e8newallm/OSDev/tmp/Interrupts.o /home/e8newallm/OSDev/tmp/Kernel.o -T /home/e8newallm/OSDev/src/Linker.lin
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-objcopy -O elf32-i386 /home/e8newallm/OSDev/tmp/Kernel.elf /mnt/OSDevMount/Kernel.elf
	sync

asm:
	clear
	clear
	/home/e8newallm/OSDev/CrossCompiler/x86_64-elf/bin/objdump -d /mnt/OSDevMount/Kernel.elf -m i386:x86-64
	
headers:
	clear
	clear
	/home/e8newallm/OSDev/CrossCompiler/x86_64-elf/bin/objdump -x /mnt/OSDevMount/Kernel.elf
	
debug: 
	clear
	clear
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-g++ -o /home/e8newallm/OSDev/tmp/Kernel.o -c /home/e8newallm/OSDev/src/Kernel.cpp -ffreestanding --no-exceptions -fpermissive -std=c++14 -m64 -mno-red-zone
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Stage2.o -c /home/e8newallm/OSDev/src/Stage2.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Interrupts.o -c /home/e8newallm/OSDev/src/Interrupts/Interrupts.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-as -o /home/e8newallm/OSDev/tmp/Switch.o -c /home/e8newallm/OSDev/src/Switch.s
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-ld -o /home/e8newallm/OSDev/tmp/Kernel.elf /home/e8newallm/OSDev/tmp/Stage2.o /home/e8newallm/OSDev/tmp/Switch.o /home/e8newallm/OSDev/tmp/Interrupts.o /home/e8newallm/OSDev/tmp/Kernel.o -T /home/e8newallm/OSDev/src/Linker.lin
	/home/e8newallm/OSDev/CrossCompiler/bin/x86_64-elf-objcopy -O elf32-i386 /home/e8newallm/OSDev/tmp/Kernel.elf /mnt/OSDevMount/Kernel.elf
	sync

mount:
	sudo mount /home/e8newallm/OSDev/VM/OSHDD.vhd /mnt/OSDevMount/

run:
	bochs -qf /home/e8newallm/OSDev/VM/BochsConfig

kvm:
	kvm -drive file="../VM/OSHDD.vhd",if=none,id=Disk,format=raw -device ich9-ahci,id=ahci -device ide-drive,drive=Disk,bus=ahci.0
