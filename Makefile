# =========================
# Compilers
# =========================
CC32 = i686-elf-gcc
CC64 = x86_64-elf-gcc
AS32 = i686-elf-as
AS64 = x86_64-elf-as

# =========================
# Flags
# =========================
CFLAGS_COMMON = -Iinclude -O2 -ffreestanding -Wall -Wextra
CFLAGS_BOOT32 = $(CFLAGS_COMMON) -mno-sse -mno-sse2 -mno-mmx -msoft-float -fno-builtin
CFLAGS_KERNEL64 = $(CFLAGS_COMMON) -mno-red-zone -mcmodel=kernel

ASFLAGS_COMMON = -Iinclude
ASFLAGS_BOOT32 = $(ASFLAGS_COMMON)
ASFLAGS_KERNEL64 = $(ASFLAGS_COMMON)

LDFLAGS_BOOT32 = -T boot32/linker.ld -ffreestanding -nostdlib
LDFLAGS_KERNEL64 = -T kernel64/linker.ld -ffreestanding -mno-red-zone -nostdlib
LDFLAGS_BOOT32_POSTFIX = -lgcc
LDFLAGS_KERNEL64_POSTFIX = -lgcc

# =========================
# Source discovery
# =========================

# Boot32 sources
BOOT32_C_SRCS := $(wildcard boot32/*.c)
BOOT32_S_SRCS := $(wildcard boot32/*.s)

# Kernel64 sources
KERNEL64_C_SRCS := $(wildcard kernel64/*.c)
KERNEL64_S_SRCS := $(wildcard kernel64/*.s)

# Common sources (drivers etc.)
# COMMON_C_SRCS := $(wildcard common/**/*.c)
# COMMON_S_SRCS := $(wildcard common/**/*.s)

COMMON_C_SRCS := $(shell find common -name "*.c")
COMMON_S_SRCS := $(shell find common -name "*.s")

# =========================
# Object mapping
# =========================

BOOT32_OBJS := \
	$(patsubst boot32/%.c, build/boot32/%.o, $(BOOT32_C_SRCS)) \
	$(patsubst boot32/%.s, build/boot32/%.o, $(BOOT32_S_SRCS)) \
	$(patsubst common/%.c, build/boot32/common/%.o, $(COMMON_C_SRCS)) \
	$(patsubst common/%.s, build/boot32/common/%.o, $(COMMON_S_SRCS))

KERNEL64_OBJS := \
	$(patsubst kernel64/%.c, build/kernel64/%.o, $(KERNEL64_C_SRCS)) \
	$(patsubst kernel64/%.s, build/kernel64/%.o, $(KERNEL64_S_SRCS)) \
	$(patsubst common/%.c, build/kernel64/common/%.o, $(COMMON_C_SRCS)) \
	$(patsubst common/%.s, build/kernel64/common/%.o, $(COMMON_S_SRCS))

# =========================
# Targets
# =========================

all: build/boot32.elf build/kernel64.elf

# =========================
# Linking
# =========================

build/boot32.elf: $(BOOT32_OBJS)
	$(CC32) $(LDFLAGS_BOOT32) $^ $(LDFLAGS_BOOT32_POSTFIX) -o $@

build/kernel64.elf: $(KERNEL64_OBJS)
	$(CC64) $(LDFLAGS_KERNEL64) $^ $(LDFLAGS_KERNEL64_POSTFIX) -o $@

# =========================
# Generic compile rules
# =========================

# C files (boot32)
build/boot32/%.o: boot32/%.c
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS_BOOT32) -c $< -o $@

# ASM files (boot32)
build/boot32/%.o: boot32/%.s
	@mkdir -p $(dir $@)
	$(AS32) $(ASFLAGS_BOOT32) -c $< -o $@

# C files (kernel64)
build/kernel64/%.o: kernel64/%.c
	@mkdir -p $(dir $@)
	$(CC64) $(CFLAGS_KERNEL64) -c $< -o $@

# ASM files (kernel64)
build/kernel64/%.o: kernel64/%.s
	@mkdir -p $(dir $@)
	$(AS64) $(ASFLAGS_KERNEL64) -c $< -o $@

# =========================
# COMMON (shared code)
# =========================

build/boot32/common/%.o: common/%.c
	@mkdir -p $(dir $@)
	$(CC32) $(CFLAGS_BOOT32) -c $< -o $@

build/kernel64/common/%.o: common/%.c
	@mkdir -p $(dir $@)
	$(CC64) $(CFLAGS_KERNEL64) -c $< -o $@

build/boot32/common/%.o: common/%.s
	@mkdir -p $(dir $@)
	$(AS32) $(ASFLAGS_BOOT32) -c $< -o $@

build/kernel64/common/%.o: common/%.s
	@mkdir -p $(dir $@)
	$(AS64) $(ASFLAGS_KERNEL64) -c $< -o $@

# =========================
# Clean
# =========================

clean:
	rm -r build/*.elf build/boot32 build/kernel64 MegaOS.iso

.PHONY: all clean
