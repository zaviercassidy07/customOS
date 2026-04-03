# !/bin/bash
nasm -f bin bootloader/boot.asm -o build/boot.bin
dd if=/dev/zero of=out/os.img bs=512 count=2880
dd if=build/boot.bin of=out/os.img conv=notrunc
