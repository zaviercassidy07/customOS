ASM=nasm
CC=gcc
LD=ld

BUILD=build
OUT=out

CFLAGS=-m64 -O0 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -nostdlib -mno-red-zone -g #g is debug
ASMFLAGS=-g

C_SOURCES=$(wildcard kernel/*.c)
ASM_SOURCES=$(wildcard kernel/*.asm)

C_OBJECTS=$(patsubst kernel/%.c, $(BUILD)/%.o, $(C_SOURCES))
ASM_OBJECTS=$(patsubst kernel/%.asm, $(BUILD)/%.o, $(ASM_SOURCES))

OBJECTS=$(ASM_OBJECTS) $(C_OBJECTS)

all: $(OUT)/os.img

clean: 
	rm -v build/*
	rm -v out/*

$(BUILD)/stage1.bin: bootloader/stage1.asm 
	$(ASM) $(ASMFLAGS) -f bin $< -o $@
$(BUILD)/stage2.bin: bootloader/stage2.asm 
	$(ASM) $(ASMFLAGS) -f bin $< -o $@

$(BUILD)/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.o: kernel/%.asm
	$(ASM) $(ASMFLAGS) -f elf64 $< -o $@

# Seperate ELF allows more debug as I can see the ELF file before converting to binary
$(BUILD)/kernel.elf: $(OBJECTS) 
	$(LD) -m elf_x86_64 -T kernel/linker.ld -o $@ $^

$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	objcopy -O binary $< $@

$(OUT)/os.img: $(BUILD)/stage1.bin $(BUILD)/stage2.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=131072 # 64mb total
	dd if=$(BUILD)/stage1.bin of=$@ count=1 conv=notrunc
	dd if=$(BUILD)/stage2.bin of=$@ count=40 seek=1 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$@ seek=41 conv=notrunc 

