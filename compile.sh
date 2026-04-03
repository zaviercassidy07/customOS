# !/bin/bash
nasm -f bin bootloader/stage1.asm -o build/stage1.bin
nasm -f bin bootloader/stage2.asm -o build/stage2.bin

dd if=/dev/zero of=out/os.img bs=512 count=2880
dd if=build/stage1.bin of=out/os.img bs=512 count=1 conv=notrunc
dd if=build/stage2.bin of=out/os.img bs=512 count=1 seek=1 conv=notrunc
