CC=x86_64-elf-gcc
AS=x86_64-elf-as
LD=x86_64-elf-ld

# ------------------------------------ BOOT32 --------------------------------- #

#LDFLAGS_BOOT32 = -m32 -nostdlib -Wl,-m,elf_i386
LDFLAGS_BOOT32 = -m elf_i386
ASFLAGS_BOOT32 = --32
CFLAGS_BOOT32 = -m32 -Iinclude -O2 -mno-sse -mno-sse2 -mno-mmx -msoft-float -fno-builtin -ffreestanding -Wall -Wextra -Werror

BOOT32_BIN = $(addprefix build/boot32/, multiboot2.o descriptor.o boot.o vga.o gdt.o idt.o isr.o isr_exceptions.o paging.o paging_as.o bootinfo.o allocator.o) 


build/boot32.elf: $(BOOT32_BIN)
	$(LD) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN)
#$(CC) $(LDFLAGS_BOOT32) -T boot32/linker.ld -o build/boot32.elf $(BOOT32_BIN) -lgcc

build/boot32/%.o: boot32/%.s
	$(AS) $(ASFLAGS_BOOT32) $< -o $@

build/boot32/%.o: boot32/%.c
	$(CC) $(CFLAGS_BOOT32) -c $< -o $@

# -------------------------------------- KERNEL64 ------------------------------ #

LDFLAGS_KERNEL64 = -nostdlib -lgcc
ASFLAGS_KERNEL64 = --64
CFLAGS_KERNEL64 = -Iinclude -O2 -ffreestanding -Wall -Wextra -Werror

KERNEL64_BIN = $(addprefix build/kernel64/, kernel_main.o entry.o)


build/kernel64.elf: $(KERNEL64_BIN)
	$(CC) $(LDFLAGS_KERNEL64) -T kernel64/linker.ld -o build/kernel64.elf $(KERNEL64_BIN)

build/kernel64/%.o: kernel64/%.s
	$(AS) $(ASFLAGS_KERNEL64) $< -o $@

build/kernel64/%.o: kernel64/%.c
	$(CC) $(CFLAGS_KERNEL64) -c $< -o $@

# ------------------------------------------------------------------------------- #
all: build/boot32.elf build/kernel64.elf

clean:
	rm -r build/boot32/*
	rm -r build/kernel64/*
	rm build/boot32.elf
	rm build/kernel64.elf
