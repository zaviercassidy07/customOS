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

start: ; this label does nothing, just here to clearly show where program starts
    call main
    jmp $ ; loop after main completes

main:
    mov si, msg
    call print
    call newLine
    mov si, msg2
    call print
    call newLine
    call newLine

    mov si, prompt
    call print

    jmp .loop
.loop:
    call getKey
    cmp al, 0x0A
    je .process
    jmp .loop
.process:
    mov si, lineBuffer
    mov ax, [bufferIndex]
    add si, ax

    mov [si], 0
    mov si, lineBuffer
    mov byte [bufferIndex], 0

    call print
    call newLine
    xor al, al

    mov si, prompt
    call print
    
    jmp .loop


print:
    lodsb ; mov al, [si] then inc si
    cmp al, 0 ; if terminate
    je .done
    call printChar
    jmp print
.done:
    ret

printChar:
    mov ah, 0x0E
    int 0x10
    ret

newLine:
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10 ; print carriage return, moves to start of line

    mov ah, 0x0E
    mov al, 0x0A
    int 0x10 ; prints newline

    ret

getKey:
    mov si, lineBuffer
    mov ax, [bufferIndex]
    add si, ax

    xor ah, ah
    int 0x16

    cmp al, 0x0D
    je .enter
    cmp al, 0x08
    je .backspace
    jne .else
.enter:
    call newLine
    ret
.backspace:
    dec byte [bufferIndex]
    mov [si], 0

    call printChar
    mov al, ' '
    call printChar
    mov al, 0x08
    call printChar
    ret
.else:
    mov [si], al
    inc byte [bufferIndex]
    call printChar
    ret

msg db "Kernel Loaded", 0
msg2 db "Hello, World!", 0

prompt db "Command: ", 0

lineBuffer times 128 db 0
bufferIndex db 0

times 4096 - ($ - $$) db 0 ; pad code