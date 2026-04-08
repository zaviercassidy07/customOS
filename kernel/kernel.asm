section .text
[bits 16]

cli ; clear interupts, it pauses the bios int codes as they're now unstable

lgdt [gdtr] ; load gdt

; enable protected mode
mov eax, cr0 ; cr0 determines whether in protected mode or not, change to 1
or eax, 1
mov cr0, eax

jmp 0x08:protected_mode_entry ; 0x08 is now segment where code is stored

[bits 32]
global protected_mode_entry ; exposes function to the linker
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

    ; define C functions
    extern print
    extern keyboardHandlerC

    mov esi, bootMsg
    push esi
    call print

    jmp $

keyboardHandler:
    pushad

    in al, 0x60 ; put scancode of keypress in al

    test al, 0x80
    jnz .done

    movzx eax, al ; move scancode to EAX
    push eax ; C functions look off top of stack for arguements
    call keyboardHandlerC ; call an external function
    add esp, 4 ; increase stack pointer by 32 bits, effectively removing the arguement from the stack

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

section .data
bootMsg db "Kernel loaded", 0
inputMsg db "Key", 0
cursorX db 0
cursorXOld db 0
cursorY db 0