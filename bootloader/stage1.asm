[org 0x7C00] ; where bios starts looking
[bits 16]   ; cpu only works in 16 bit at this point, even if it supports higher later

mov [0x7D00], dl ; store the disk index we're using

in al, 0x92 ; 0x92 is sys controller
or al, 2
out 0x92, al

xor bx, bx
mov ah, 0x42 ; long read 
mov dl, [0x7D00] ; disk
mov ds, bx
mov si, dap ; ds:si is address of read info
int 0x13

jmp 0x0000:0x8000 ; jump to where it was loaded
; we need to load a second file as this one has a limit of 512B. I don't think we're close to that but its just easier to get rid of that limit

dap:
    db 0x10 ; size, always 16b
    db 0 ; reserved
    dw 40 ; num sectors to read
    dw 0x8000 ; offset in buffer to write to
    dw 0 ; segment to write to
    dq 1 ; sector to start reading

times 510 - ($ - $$) db 0 ; writes all but last 2 bytes 0, program has to be 512 long
dw 0xAA55 ; boot loader code, if this isnt in last two bytes it will refuse to boot