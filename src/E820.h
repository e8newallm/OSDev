     /* multiboot.h - the header for Multiboot */
     /* Copyright (C) 1999, 2001  Free Software Foundation, Inc.
     
        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.
     
        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.
     
        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
     
     /* Macros. */
     
     /* The magic number for the Multiboot header. */
     #define MULTIBOOT_HEADER_MAGIC          0x1BADB002
     
     /* The flags for the Multiboot header. */
     #ifdef __ELF__
     # define MULTIBOOT_HEADER_FLAGS         0x00000003
     #else
     # define MULTIBOOT_HEADER_FLAGS         0x00010003
     #endif
     
     /* The magic number passed by a Multiboot-compliant boot loader. */
     #define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002
     
     /* The size of our stack (16KB). */
     #define STACK_SIZE                      0x4000
     
     /* C symbol format. HAVE_ASM_USCORE is defined by configure. */
     #ifdef HAVE_ASM_USCORE
     # define EXT_C(sym)                     _ ## sym
     #else
     # define EXT_C(sym)                     sym
     #endif
     
     #ifndef ASM
     /* Do not include here in boot.S. */
     
     /* Types. */
     
     /* The Multiboot header. */
     typedef struct multiboot_header
     {
       unsigned int magic;
       unsigned int flags;
       unsigned int checksum;
       unsigned int header_addr;
       unsigned int load_addr;
       unsigned int load_end_addr;
       unsigned int bss_end_addr;
       unsigned int entry_addr;
     } multiboot_header_t;
     
     /* The symbol table for a.out. */
     typedef struct aout_symbol_table
     {
       unsigned int tabsize;
       unsigned int strsize;
       unsigned int addr;
       unsigned int reserved;
     } aout_symbol_table_t;
     
     /* The section header table for ELF. */
     typedef struct elf_section_header_table
     {
       unsigned int num;
       unsigned int size;
       unsigned int addr;
       unsigned int shndx;
     } elf_section_header_table_t;
     
     /* The Multiboot information. */
     typedef struct multiboot_info
     {
       unsigned int flags;
       unsigned int mem_lower;
       unsigned int mem_upper;
       unsigned int boot_device;
       unsigned int cmdline;
       unsigned int mods_count;
       unsigned int mods_addr;
       union
       {
         aout_symbol_table_t aout_sym;
         elf_section_header_table_t elf_sec;
       } u;
       unsigned int mmap_length;
       unsigned int mmap_addr;
	   
	   unsigned int drives_length;
	   unsigned int drives_addr;
	   
	   unsigned int config_table;
	   
	   unsigned int boot_loader_name;
	   
	   unsigned int apm_table;
	   
	   unsigned int vbe_control_info;
       unsigned int vbe_mode_info;
       unsigned short vbe_mode;
       unsigned short vbe_interface_seg;
       unsigned short vbe_interface_off;
       unsigned short vbe_interface_len;
     } multiboot_info_t;
     
     /* The module structure. */
     typedef struct module
     {
       unsigned int mod_start;
       unsigned int mod_end;
       unsigned int string;
       unsigned int reserved;
     } module_t;
     
     /* The memory map. Be careful that the offset 0 is base_addr_low
        but no size. */
     typedef struct memory_map
     {
       unsigned int size;
       unsigned int base_addr_low;
       unsigned int base_addr_high;
       unsigned int length_low;
       unsigned int length_high;
       unsigned int type;
     } memory_map_t;
     
	 struct vbe_mode_info_struct
	 {
		unsigned short ModeAttr;
		unsigned char WinAAttr;
		unsigned char WinBAttr;
		unsigned short WinGran;
		unsigned short WinSize;
		unsigned short WinASeg;
		unsigned short WinBSeg;
		unsigned int WinFuncPtr;
		unsigned short BytesPerScanLine;
		unsigned short XRes;
		unsigned short YRes;
		unsigned char XCharSize;
		unsigned char YCharSize;
		unsigned char NumPlanes;
		unsigned char BitsPerPixel;
		unsigned char NumBanks;
		unsigned char MemoryModel;
		unsigned char BankSize;
		unsigned char NumImagePages;
		unsigned char Reserved;
		unsigned char RedMaskSize;
		unsigned char RedFieldPos;
		unsigned char GreenMaskSize;
		unsigned char GreenFieldPos;
		unsigned char BlueMaskSize;
		unsigned char BlueFieldPos;
		unsigned char RSVDMaskSize;
		unsigned char RSVDFieldPos;
		unsigned char DirectColorModeInfo;
		unsigned int PhysBasePtr;
	 } __attribute__((packed));
	 
     #endif /* ! ASM */
     