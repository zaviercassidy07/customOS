; placeholder kernel to test bootloader

[org 0x0000] ; no offset for this one
[bits 16]

; set up stack
mov ax, 0x950
mov ss, ax
xor sp, sp
; stack at ss*16 + sp, so this puts it at 0x9500, a bit above kernel

mov ax, 0x900
mov ds, ax ; address to read = ds * 16 + offset, so this sets ds to 0x9000, where we are

mov si, msg ; put msg in source

.printChar:
    lodsb ; mov al, [si] then inc si
    cmp al, 0 ; if terminate
    je .stackTest
    mov ah, 0x0E ; write arguement
    int 0x10 ; bios call
    jmp .printChar

.stackTest:
    mov ax, 't'
    push ax
    mov ax, 's'
    push ax
    mov ax, 'e'
    push ax
    mov ax, 't'
    push ax
    mov ax, 10
    push ax

    pop ax
    mov ah, 0x0E
    int 0x10
    pop ax
    mov ah, 0x0E
    int 0x10
    pop ax
    mov ah, 0x0E
    int 0x10
    pop ax
    mov ah, 0x0E
    int 0x10
    pop ax
    mov ah, 0x0E
    int 0x10

    jmp .done

.done:
    jmp $ ; loop

msg db "Hello, World!", 0

times 512 - ($ - $$) db 0 ; pad code