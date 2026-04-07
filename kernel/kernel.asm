[org 0x9000]
[bits 16]


cli ; clear interupts, it pauses the bios int codes as they're now unstable

lgdt [gdtr] ; load gdt

; enable protected mode
mov eax, cr0 ; cr0 determines whether in protected mode or not, change to 1
or eax, 1
mov cr0, eax

jmp 0x08:protected_mode_entry ; 0x08 is now segment where code is stored

[bits 32]
protected_mode_entry:
    ; reload data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000   ; initialize stack

    mov esi, bootMsg
    call print
    call newLine

    jmp $

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

    add eax, [vidMem]
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

gdt_start:
    ; bits are 2 char, bytes in order are base-high, flags and limit-high, access perms, base mid, base mid, base low, limit mid, limit low
    dq 0x0000000000000000 ; null
    dq 0x00CF9A000000FFFF ; code (base=0, 4 GB)
    dq 0x00CF92000000FFFF ; data (base=0, 4 GB)
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; length of gdt - 1
    dd gdt_start ; start address

bootMsg db "Kernel loaded", 0

vidMem dd 0xB8000
cursorX db 0
cursorY db 0

times 4096 - ($ - $$) db 0