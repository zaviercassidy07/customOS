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

    call initPaging

    call initPIC
    call initIDT

    ; define C functions
    extern main
    extern keyboardHandler_c

    call main

    jmp $

keyboardHandler_asm:
    pushad

    in al, 0x60 ; put scancode of keypress in al

    test al, 0x80 ; ignore if key release
    jnz .done

    movzx eax, al ; move scancode to EAX
    push eax ; C functions look off top of stack for arguements
    call keyboardHandler_c ; call an external function
    add esp, 4 ; increase stack pointer by 32 bits, effectively removing the arguement from the stack

    jmp .done
.done:
    mov al, 0x20
    out 0x20, al ; end interupt, program continues

    popad
    iret

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
    mov eax, keyboardHandler_asm

    mov word [idt_start + 0x21*8], ax ; move lower half of keyboardHandler address to start of int code
    shr eax, 16 ; move higher half of eax to lower half
    mov word [idt_start + 0x21*8 + 6], ax ; move higher half of address now to end of int code

    mov word [idt_start + 0x21*8 + 2], 0x08 ; fill in middle info
    mov byte [idt_start + 0x21*8 + 4], 0
    mov byte [idt_start + 0x21*8 + 5], 0x8E

    lidt [idtr] ; load idt
    sti ; restart interupts now that my own interupt table is loaded and we're in 32bit

    ret

; current paging is 1:1 mapping of virtual to physical memory, really no benefit to paging at this point but it allows expansion later
initPaging:
    mov ecx, 1024 ; set up loop to do it 1024 times
    mov ebx, 0
    mov edi, page_table
.fillTable:
    mov eax, ebx
    or eax, 0x3
    mov [edi], eax

    add edi, 4 ; move up 4 bytes, the length of one entry in table
    add ebx, 0x1000 ; +4096
    loop .fillTable ; ecx--, repeat until ecx = 0

    mov eax, page_table ; page table goes into page directory same way an entry goes in table
    or eax, 0x3
    mov [page_directory], eax

    mov eax, page_directory
    mov cr3, eax ; page directory gets stored in cr3

    mov eax, cr0
    or eax, 0x80000000 ; when paging bit in cr0 is on, it reads directory from cr3 and enabled paging
    mov cr0, eax

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

align 4096 ; moves address forward until divisible by 4096, these tables need to be on even address
page_directory: ; contains pointer to tables
    times 1024 dd 0

align 4096
page_table:
    times 1024 dd 0