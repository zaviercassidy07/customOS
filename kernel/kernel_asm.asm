section .text
[bits 64]

global start
start:
    mov rsp, stack_top
    and rsp, -16

    call initPIC
    call initIDT

    extern main
    extern keyboardHandler_c
    extern print
    extern printHex
    extern newLine
    extern clearScreen

    call main

    jmp $

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

    lidt [idtr]
    sti

    ret

addIDTEntry:
    mov rbx, rdi
    mov rdx, rax

    shl rbx, 4 ; multiply by 16 as each index is 16 bytes long (shl is shuffle left)
    add rbx, idt_start ; get start address

    ; offset low
    mov ax, dx
    mov word [rbx], ax ; store first 16 bits of address

    ; selector
    mov word [rbx + 2], 0x18 ; where in the GDT table the handler function is

    ; IST (0) + reserved
    mov byte [rbx + 4], 0 ; reserved byte for standards reasons

    ; type = interrupt gate
    mov byte [rbx + 5], 0x8E ; How the interupt is handled, different values can allow recursive interupts etc

    ; offset mid
    mov rax, rdx
    shr rax, 16 ; shuffle right 16, ax now has next 16 bits along
    mov word [rbx + 6], ax ; store another 16 bits

    ; offset high
    shr rax, 16 ; shuffle another 16 bits, eax will have last 32 bits of handler address
    mov dword [rbx + 8], eax

    ; zero
    mov dword [rbx + 12], 0 ; terminating character

    ret

gp_fault:
    mov word [0xB8000], 0x4F47 ; GP
    mov word [0xB8002], 0x4F50

    cli
    hlt
page_fault:
    ;call clearScreen

    mov word [0xB8000], 0x4F50 ; PAGE
    mov word [0xB8002], 0x4F41
    mov word [0xB8004], 0x4F47
    mov word [0xB8006], 0x4F45

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
    in al, 0x60
    
    cmp al, 0x80 ; if greater than 0x80, probably key release so ignore. May have to change this later
    ja .done

    movzx rdi, al
    call keyboardHandler_c

.done:
    mov al, 0x20
    out 0x20, al

    pop rax
    iretq

align 16
idt_start:
    times 512 dq 0 ; 256 64 bit entries
idt_end:

idtr: ; same as gdtr
    dw idt_end - idt_start - 1
    dq idt_start

section .bss

align 16
stack_bottom:
    resb 8192
stack_top: