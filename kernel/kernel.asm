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

    call initPIC
    call initIDT

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

    add eax, 0xB8000
    mov edi, eax

    pop ax

    mov ah, 0x0F
    mov [edi], ax

    inc byte [cursorX]

    cmp byte [cursorX], 80
    jl .done ; jump if less then

    mov byte [cursorXOld], 79 ; store old X in case we go back a line
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

keyboardHandler:
    pushad

    in al, 0x60 ; put scancode of keypress in al

    test al, 0x80
    jnz .done

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
    mov al, [cursorX]
    mov byte [cursorXOld], al ; store old X in case we go back a line
    
    call newLine
    jmp .done
.back:
    cmp byte [cursorX], 0
    je .backLine

    dec byte [cursorX]
    mov al, 0
    call printChar
    dec byte [cursorX]

    call movCursor

    jmp .done
.backLine:
    cmp byte [cursorY], 0 ; ignore if cursor at 0, 0
    je .done

    dec byte [cursorY]
    mov al, [cursorXOld]
    mov byte [cursorX], al

    cmp byte [cursorX], 79
    je .endLine

    call movCursor

    jmp .done
.endLine:
    mov al, 0
    call printChar

    dec byte [cursorY]
    mov byte [cursorX], 79
    call movCursor

    jmp .done
.done:
    mov al, 0x20
    out 0x20, al ; end interupt, program continues

    popad
    iret

keyMap db 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 59,  39, '`', 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 32

initPIC:
    mov al, 0x11 ; 0x11 is init sequence, tell command for each we are doing init sequence
    out 0x20, al
    out 0xA0, al

    mov al, 0x20 ; this block sets offset, we have to set offset as default range is reserved by CPU for other stuff, so we move ints up
    out 0x21, al
    mov al, 0x28
    out 0xA1, al

    mov al, 0x04 ; this block sets up the slave PIC to talk to the master PIC right, slave connects to master IRQ2
    out 0x21, al
    mov al, 0x02
    out 0xA1, al

    mov al, 0x01 ; set PIC to x86 mode
    out 0x21, al
    out 0xA1, al

    mov al, 0xFD ; enable only keyboard interupts
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al

    ret

initIDT:
    ; write keyboard interrupt code
    mov eax, keyboardHandler

    mov word [idt_start + 0x21*8], ax ; move lower half of keyboardHandler address to start of int code
    shr eax, 16 ; move higher half of eax to lower half
    mov word [idt_start + 0x21*8 + 6], ax ; move higher half of address now to end of int code

    mov word [idt_start + 0x21*8 + 2], 0x08 ; fill in middle info
    mov byte [idt_start + 0x21*8 + 4], 0
    mov byte [idt_start + 0x21*8 + 5], 0x8E

    lidt [idtr] ; load idt
    sti ; restart interupts now that my own interupt table is loaded and we're in 32bit

    ret

gdt_start:
    ; bytes are 2 char, bytes in order are base-high, flags and limit-high, access perms, base mid, base mid, base low, limit mid, limit low
    dq 0x0000000000000000 ; null
    dq 0x00CF9A000000FFFF ; code (base=0, 4 GB)
    dq 0x00CF92000000FFFF ; data (base=0, 4 GB)
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; length of gdt - 1
    dd gdt_start ; start address

idt_start:
    times 256 dq 0 ; pad to 256 entries
idt_end:

idtr:
    dw idt_end - idt_start - 1 ; length of idt - 1
    dd idt_start ; same as gdt

bootMsg db "Kernel loaded", 0
inputMsg db "Key", 0

cursorX db 0
cursorXOld db 0
cursorY db 0

times 4096 - ($ - $$) db 0