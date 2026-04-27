[bits 64]
section .text

global start
start:
    mov rax, stack_top
    mov rsp, rax
    and rsp, -16

    mov rax, gdtr
    lgdt [rax] ; we need to reload the gdt as it has to be in mapped memory, and jump to the new code segment
    ; we'll also use this chance to get rid of the 32 bit entries in gdt
    
    push 0x08
    mov rax, refreshGDT
    push rax
    retfq ; far return quad, gets address of stack which is why we push segment and function. Only way to do this in long mode so im told

refreshGDT:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax ; reinit registers

    call initPIC
    call initIDT

    call removeIdentityMap

    extern main
    extern keyboardHandler_c
    extern print
    extern printHex
    extern newLine
    extern clearScreen

    call main

    jmp $

removeIdentityMap:
    mov rax, 0xFFFFFF7FBFDFE000 ; rdi has physical address for pml4, add virtual offset
    mov qword [rax], 0

    mov rax, cr3
    mov cr3, rax ; reload cr3

    ret

initPIC:
    mov al, 0x11 ; init begin command
    out 0x20, al ; master
    out 0xA0, al ; slave

    mov al, 0x20 ; add offset, put them just above CPU exception (IRQ0 becomes 0x20)
    out 0x21, al
    mov al, 0x28
    out 0xA1, al

    mov al, 0x04 ; tells master that slave is on IRQ2 (0x04 is 0b100, or 2nd bit starting from 0)
    out 0x21, al
    mov al, 0x02 ; directly says connected to master's IRQ2
    out 0xA1, al

    mov al, 0x01 ; set mode, this is default mode as others are fairly useless on modern hardware
    out 0x21, al
    out 0xA1, al

    mov al, 0xFD ; mask interupts, only allow through what we use (IRQ1 for keyboard)
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al

    ret

initIDT:
    ; double fault, exception 8
    mov rax, double_fault
    mov rdi, 0x08
    call addIDTEntry

    ; init gp fault, exception 13 (0x0D)
    mov rax, gp_fault
    mov rdi, 0x0D
    call addIDTEntry

    ; init page fault, exception 14 (0x0E)
    mov rax, page_fault
    mov rdi, 0x0E
    call addIDTEntry

    ; keyboard input IRQ1 mapped to 0x21
    mov rax, keyboardHandler_asm
    mov rdi, 0x21
    call addIDTEntry

    mov rax, idtr
    lidt [rax]
    sti

    ret

addIDTEntry:
    ; rdi = idt index
    ; rax = function addr

    shl rdi, 4 ; multiply by 16 as each index is 16 bytes long (shl is shuffle left)
    mov rcx, idt_start
    add rdi, rcx ; get start address

    ; offset low
    mov word [rdi], ax ; store first 16 bits of address

    ; selector
    mov word [rdi + 2], 0x08 ; where in the GDT table the handler function is

    mov byte [rdi + 4], 0 ; reserved byte for standards reasons

    ; type = interrupt gate
    mov byte [rdi + 5], 0x8E ; How the interupt is handled, different values can allow recursive interupts etc

    ; offset mid
    shr rax, 16 ; shuffle right 16, ax now has next 16 bits along
    mov word [rdi + 6], ax ; store another 16 bits

    ; offset high
    shr rax, 16 ; shuffle another 16 bits, eax will have last 32 bits of handler address
    mov dword [rdi + 8], eax

    mov dword [rdi + 12], 0 ; terminating characters

    ret

double_fault:
    mov rax, 0xFFFFFF80000B8000
    mov word [rax], 0x4F44 ; DF
    add rax, 2
    mov word [rax], 0x4F46

    cli
    hlt

gp_fault:
    mov rax, 0xFFFFFF80000B8000
    mov word [rax], 0x4F47 ; GP
    add rax, 2
    mov word [rax], 0x4F50

    cli
    hlt
page_fault:
    ;call clearScreen

    mov rax, 0xFFFFFF80000B8000
    mov word [rax], 0x4F50 ; PAGE
    add rax, 2
    mov word [rax], 0x4F41
    add rax, 2
    mov word [rax], 0x4F47
    add rax, 2
    mov word [rax], 0x4F45

    call newLine
    pop rdi
    mov rsi, 1
    call printHex ; print specific error, 0x2 is trying to access unmapped page
    call newLine
    pop rdi
    mov rsi, 1
    call printHex ; print EXACTLY what the CPU was trying to do when fault occured
    call newLine
    mov rdi, cr2
    mov rsi, 1
    call printHex ; print address that causes fault

    mov al, 0x20
    out 0x20, al
    cli
    hlt

keyboardHandler_asm:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi

    in al, 0x60
    
    cmp al, 0x80 ; if greater than 0x80, probably key release so ignore. May have to change this later
    ja .done

    movzx rdi, al
    call keyboardHandler_c

.done:
    mov al, 0x20
    out 0x20, al

    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq

section .data

gdt_start:
    ; bytes are 2 char, bytes in order are base-high, flags and limit-high, access perms, base mid, base mid, base low, limit mid, limit low
    dq 0x0000000000000000 ; null
    dq 0x00A09A0000000000 ; code 64bit
    dq 0x00A0920000000000 ; data 64bit
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; length of gdt - 1
    dq gdt_start ; start address

idtr: ; same as gdtr
    dw idt_end - idt_start - 1
    dq idt_start

section .bss

align 16
idt_start:
    resb 4096 ; 512 64 bit entries
idt_end:

align 16
stack_bottom:
    resb 32768
stack_top: