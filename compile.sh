# !/bin/bash
nasm -f bin bootloader/stage1.asm -o build/stage1.bin
nasm -f bin bootloader/stage2.asm -o build/stage2.bin

nasm -f elf32 kernel/kernel.asm -o build/kernel_asm.o
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c kernel/keyboardHandler.c -o build/keyboardHandler.o
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c kernel/printUtils.c -o build/printUtils.o
ld -m elf_i386 -T kernel/linker.ld --oformat binary -o build/kernel.bin build/kernel_asm.o build/keyboardHandler.o build/printUtils.o

dd if=/dev/zero of=out/os.img bs=512 count=2880
dd if=build/stage1.bin of=out/os.img bs=512 count=1 conv=notrunc
dd if=build/stage2.bin of=out/os.img bs=512 count=4 seek=1 conv=notrunc
dd if=build/kernel.bin of=out/os.img bs=512 count=8 seek=5 conv=notrunc