[org 0x7C00] ; where bios starts looking
[bits 16]   ; cpu only works in 16 bit at this point, even if it supports higher later

xor ax, ax ; zero register 
mov ds, ax ; set starting point to our code at 0 relative to true point, essentially sets it to 7C00

mov si, msg

.print_char:
    lodsb ; mov al, si then inc si
    cmp al, 0 ; if we reach terminating character, move to the loop
    je .done
    mov ah, 0x0E ; arguements and print char
    int 0x10
    jmp .print_char

.done:
    jmp $ ; loops program

msg db "Hello, world!", 0 ; store data, should be after code if not in .data tag

times 510 - ($ - $$) db 0 ; writes all but last 2 bytes 0, program has to be 512 long
dw 0xAA55 ; boot loader code, if this isnt in last two bytes it will refuse to boot