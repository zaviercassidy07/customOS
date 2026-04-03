; placeholder kernel to test bootloader

[org 0x0000] ; no offset for this one
[bits 16]

mov ax, 0x900
mov ds, ax ; address to read = ds * 16 + offset, so this sets ds to 0x9000, where we are

mov si, msg ; put msg in source

.print_char:
    lodsb ; mov al, [si] then inc si
    cmp al, 0 ; if terminate
    je .done
    mov ah, 0x0E ; write arguement
    int 0x10 ; bios call
    jmp .print_char

.done:
    jmp $ ; loop

msg db "Hello, World!", 0

times 512 - ($ - $$) db 0 ; pad code