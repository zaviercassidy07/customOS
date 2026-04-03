# !/bin/bash
nasm -f bin bootloader/stage1.asm -o build/stage1.bin
nasm -f bin bootloader/stage2.asm -o build/stage2.bin

nasm -f bin kernel/kernel.asm -o build/kernel.bin

dd if=/dev/zero of=out/os.img bs=512 count=2880
dd if=build/stage1.bin of=out/os.img bs=512 count=1 conv=notrunc
dd if=build/stage2.bin of=out/os.img bs=512 count=4 seek=1 conv=notrunc
dd if=build/kernel.bin of=out/os.img bs=512 count=1 seek=5 conv=notrunc