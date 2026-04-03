[org 0x7C00] ; where bios starts looking
[bits 16]   ; cpu only works in 16 bit at this point, even if it supports higher later

xor ax, ax ; zero register 
mov ds, ax
mov es, ax ; zero the two
mov bx, 0x8000 ; bx contains destination address to load into

mov ah, 0x02 ; code for read operation
mov al, 4 ; number of sectors to read
mov ch, 0 ; cylinder number to read
mov cl, 2 ; sector number, 0 is index, 1 is this one, 2 is next
mov dh, 0 ; head number to read
int 0x13 ; bios call for disk operation

jmp 0x8000 ; jump to where it was loaded
; we need to load a second file as this one has a limit of 512B. I don't think we're close to that but its just easier to get rid of that limit

times 510 - ($ - $$) db 0 ; writes all but last 2 bytes 0, program has to be 512 long
dw 0xAA55 ; boot loader code, if this isnt in last two bytes it will refuse to boot