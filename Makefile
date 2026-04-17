CC=x86_64-elf-gcc
AS=x86_64-elf-as
LD=x86_64-elf-ld

#LDFLAGS_BOOT32 = -m32 -nostdlib -Wl,-m,elf_i386
LDFLAGS_BOOT32 = -m elf_i386
ASFLAGS_BOOT32 = --32
CFLAGS_BOOT32 = -m32 -Iinclude -O2 -ffreestanding -Wall -Wextra

BOOT32_BIN = $(addprefix build/boot32/, multiboot2.o gdt_as.o boot.o vga.o gdt.o)

build/boot32.elf: $(BOOT32_BIN)
	$(LD) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN)
#$(CC) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN) -lgcc

build/boot32/%.o: boot32/%.s
	$(AS) $(ASFLAGS_BOOT32) $< -o $@

build/boot32/%.o: boot32/%.c
	$(CC) $(CFLAGS_BOOT32) -c $< -o $@

#multiboot2.o: boot32/multiboot2.s
#	$(AS) $(ASFLAGS_BOOT32) boot32/multiboot2.s -o multiboot2.o

#boot.o: boot32/boot.c
#	$(CC) $(CFLAGS_BOOT32) -c boot32/boot.c -o boot.o


clean:
	rm -r build/boot32/*
	rm build/boot32.elf
