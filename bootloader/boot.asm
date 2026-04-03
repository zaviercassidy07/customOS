[org 0x7C00] ; where bios starts looking
[bits 16]   ; cpu only works in 16 bit at this point, even if it supports higher later

mov ah, 0x0E ; bios write function
mov al, 'H' ; data to be written
int 0x10 ; equivalent of syscall

jmp $ ; loops program

times 510 - ($ - $$) db 0 ; writes all but last 2 bytes 0, program has to be 512 long
dw 0xAA55 ; boot loader code, if this isnt in last two bytes it will refuse to boot