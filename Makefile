CC64=x86_64-elf-gcc
AS64=x86_64-elf-as
LD64=x86_64-elf-ld

CC32=i686-elf-gcc
AS32=i686-elf-as
LD32=i686-elf-ld

# ------------------------------------ BOOT32 --------------------------------- #

LDFLAGS_BOOT32 = -nostdlib -lgcc
#LDFLAGS_BOOT32 = -m elf_i386
ASFLAGS_BOOT32 = --32
CFLAGS_BOOT32 = -Iinclude -O2 -mno-sse -mno-sse2 -mno-mmx -msoft-float -fno-builtin -ffreestanding -Wall -Wextra

BOOT32_BIN = $(addprefix build/boot32/, multiboot2.o descriptor.o boot.o vga.o gdt.o \
			 idt.o isr.o isr_exceptions.o paging.o paging_as.o bootinfo.o allocator.o\
			 elf.o) 


build/boot32.elf: $(BOOT32_BIN)
#	$(LD) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN)
	$(CC32) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN)

build/boot32/%.o: boot32/%.s
	$(AS32) $(ASFLAGS_BOOT32) $< -o $@

build/boot32/%.o: boot32/%.c
	$(CC32) $(CFLAGS_BOOT32) -c $< -o $@

# -------------------------------------- KERNEL64 ------------------------------ #

LDFLAGS_KERNEL64 = -nostdlib -mno-red-zone -lgcc
ASFLAGS_KERNEL64 = --64
CFLAGS_KERNEL64 = -Iinclude -O2 -ffreestanding -mno-red-zone -Wall -Wextra

KERNEL64_BIN = $(addprefix build/kernel64/, kernel_main.o entry.o)


build/kernel64.elf: $(KERNEL64_BIN)
	$(CC64) $(LDFLAGS_KERNEL64) -T kernel64/linker.ld -o build/kernel64.elf $(KERNEL64_BIN)

build/kernel64/%.o: kernel64/%.s
	$(AS64) $(ASFLAGS_KERNEL64) $< -o $@

build/kernel64/%.o: kernel64/%.c
	$(CC64) $(CFLAGS_KERNEL64) -c $< -o $@

# ------------------------------------------------------------------------------- #
all: build/boot32.elf build/kernel64.elf

clean:
	rm -r build/boot32/*
	rm -r build/kernel64/*
	rm build/boot32.elf
	rm build/kernel64.elf
