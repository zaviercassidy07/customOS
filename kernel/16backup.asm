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

    xor al, al

    mov di, echo
    call compareString
    cmp al, 1
    je .echo
    
    mov si, lineBuffer
    mov di, clear
    call compareString
    cmp al, 1
    je .clear

    call newLine
    mov si, prompt
    call print

    jmp .loop
.echo:
    mov si, lineBuffer
    add si, 5
    call print

    call newLine
    mov si, prompt
    call print

    jmp .loop
.clear:
    mov ah, 6
    mov al, 0
    mov bh, 0x07
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    mov ah, 0x02
    mov bh, 0
    mov dh, 0
    mov dl, 0
    int 0x10

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

compareString:
    mov al, [si]
    mov bl, [di]

    cmp al, 0
    je .done
    cmp al, 0x20
    je .done

    cmp al, bl
    jne .notEqual

    inc si
    inc di
    
    jmp compareString
.notEqual:
    mov al, 0
    ret
.equal:
    mov al, 1
    ret
.done:
    cmp al, bl
    je .equal
    mov al, 0
    ret

msg db "Kernel Loaded", 0
msg2 db "Hello, World!", 0

prompt db "Command: ", 0

echo db "echo "
clear db "clear", 0

lineBuffer times 128 db 0
bufferIndex db 0

OLD KEYBOARD keyboardHandler
    movzx ebx, al
    mov al, [keyMap + ebx] ; add scancode to memory of keyMap, returns coresponding key into al
    cmp al, 13
    je .enter
    cmp al, 8
    je .back
    cmp al, 0 ; if key not in keymap
    je .done

    call printChar

    jmp .done
.enter:
    call newLine
    jmp .done
.back:
    cmp byte [cursorX], 0
    je .done

    dec byte [cursorX]
    mov al, 0
    call printChar
    dec byte [cursorX]

    call movCursor

keyMap db 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 59,  39, '`', 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 32

print:
    mov al, [esi]
    inc esi

    cmp al, 0
    je .done

    call printChar
    jmp print
.done:
    ret

printChar:
    push ax

    movzx eax, byte [cursorY]
    mov ecx, 80
    mul ecx

    movzx ebx, byte [cursorX]
    add eax, ebx

    shl eax, 1 ; easy x2

    add eax, 0xB8000
    mov edi, eax

    pop ax

    mov ah, 0x0F
    mov [edi], ax

    inc byte [cursorX]

    cmp byte [cursorX], 80
    jl .done ; jump if less then

    mov byte [cursorX], 0
    inc byte [cursorY]

    call movCursor
    ret
.done:
    call movCursor
    ret

movCursor:
    mov al, byte [cursorY]
    mov bl, 80
    mul bl
    movzx bx, byte [cursorX]
    add ax, bx
    mov bx, ax

    mov dx, 0x3D4 ; selector port, we write the register we actually want to talk to here
    mov al, 0x0F ; the register we want to talk to
    out dx, al ; send data in al out port saved at dx

    mov dx, 0x3D5 ; receiver port, we send data here which is then sent to the register previously defined
    mov al, bl ; the data to send
    out dx, al ; send the data

    ; folowing code repeats this again, vga is old so we send high and low half of register seperately
    mov dx, 0x3D4
    mov al, 0x0E
    out dx, al

    mov dx, 0x3D5
    mov al, bh
    out dx, al

    ret

newLine:
    inc byte [cursorY]
    mov byte [cursorX], 0

    call movCursor

    ret