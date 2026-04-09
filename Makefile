ASM=nasm
CC=gcc
LD=ld

BUILD=build
OUT=out

CFLAGS=-m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib

C_SOURCES=$(wildcard kernel/*.c)
ASM_SOURCES=$(wildcard kernel/*.asm)

C_OBJECTS=$(patsubst kernel/%.c, $(BUILD)/%.o, $(C_SOURCES))
ASM_OBJECTS=$(patsubst kernel/%.asm, $(BUILD)/%.o, $(ASM_SOURCES))

OBJECTS=$(ASM_OBJECTS) $(C_OBJECTS)

all: $(OUT)/os.img

$(BUILD)/stage1.bin: bootloader/stage1.asm 
	$(ASM) -f bin $< -o $@
$(BUILD)/stage2.bin: bootloader/stage2.asm 
	$(ASM) -f bin $< -o $@

$(BUILD)/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.o: kernel/%.asm
	$(ASM) -f elf32 $< -o $@

$(BUILD)/kernel.bin: $(OBJECTS) 
	$(LD) -m elf_i386 -T kernel/linker.ld --oformat binary -o $(BUILD)/kernel.bin $^

$(OUT)/os.img: $(BUILD)/stage1.bin $(BUILD)/stage2.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BUILD)/stage1.bin of=$@ count=1 conv=notrunc
	dd if=$(BUILD)/stage2.bin of=$@ count=4 seek=1 conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$@ count=8 seek=5 conv=notrunc 

