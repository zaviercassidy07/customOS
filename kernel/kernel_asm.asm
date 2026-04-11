section .text
[bits 16]
global start ; export function to linker
start:
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

    call initPaging

    jmp prepLong

initPaging:
    mov edi, PD_start
    mov ebx, 0x83
.fillPD:
    mov eax, ebx
    mov [edi], eax ; fill top half with data
    mov dword [edi + 4], 0 ; fill higher half with 0s

    add ebx, 0x200000 ; add two Mb
    add edi, 8 ; add 64 bits or one address to pointer

    cmp edi, PD_end ; if not done, loop back
    jne .fillPD

    mov eax, PD_start
    or eax, 0x03
    mov [PDPT], eax
    mov dword [PDPT + 4], 0 ; again, populating table, fill half with 0s

    mov eax, PDPT
    or eax, 0x03
    mov [PML4], eax
    mov dword [PML4 + 4], 0

    ret

    
prepLong:
    mov ebx, cr0
    and ebx, ~(1 << 31) ; make sure paging bit is disabled (~ is not). must be disabled before we do this
    mov cr0, ebx

    mov edx, cr4
    or edx, (1 << 5) ; enable PAE bit
    mov cr4, edx

    mov ecx, 0xC0000080
    rdmsr ; read 64 bit register in ecx, split into edx:eax
    or eax, (1 << 8) ; turn on long mode bit
    wrmsr ; write into 64 bit register

    mov eax, PML4
    mov cr3, eax ; store address of PML4

    mov ebx, cr0
    or ebx, (1 << 31); enable paging again
    mov cr0, ebx

    jmp 0x18:long_mode_entry

[bits 64]
long_mode_entry:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax ; reinit registers

    mov rsp, stack_top
    and rsp, -16

    call initPIC
    call initIDT

    mov rbx, 0x0F540F530F450F54 ; TEST encoded
    push rbx
    pop rax
    mov QWORD [0xB8000], rax ; print

    extern main
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

    shl rbx, 4 ; multiply by 16
    add rbx, idt_start ; get start address

    ; offset low
    mov ax, dx
    mov word [rbx], ax

    ; selector
    mov word [rbx + 2], 0x18

    ; IST (0) + reserved
    mov byte [rbx + 4], 0

    ; type = interrupt gate
    mov byte [rbx + 5], 0x8E

    ; offset mid
    mov rax, rdx
    shr rax, 16 ; NEED TO FIGURE OUT WHAT THIS DOES EXACTLY, ACTUALLY NEED TO CHECK WHOLE DATA LAYOUT HERE
    mov word [rbx + 6], ax

    ; offset high
    shr rax, 16
    mov dword [rbx + 8], eax

    ; zero
    mov dword [rbx + 12], 0

    ret

gp_fault:
    mov word [0xB8000], 0x0F47 ; G
    cli
    hlt
page_fault:
    mov word [0xB8000], 0x0F50 ; P
    mov al, 0x20
    out 0x20, al
    cli
    hlt

keyboardHandler_asm:
    push rax
    in al, 0x60
    
    mov word [0xB8000], 0x0F4B ; K

    mov al, 0x20
    out 0x20, al

    pop rax
    iretq

gdt_start:
    ; bytes are 2 char, bytes in order are base-high, flags and limit-high, access perms, base mid, base mid, base low, limit mid, limit low
    dq 0x0000000000000000 ; null
    dq 0x00CF9A000000FFFF ; code 32bit (base=0, 4 GB)
    dq 0x00CF92000000FFFF ; data 32bit (base=0, 4 GB)
    dq 0x00A09A0000000000 ; code 64bit
    dq 0x00A0920000000000 ; data 64bit
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; length of gdt - 1
    dq gdt_start ; start address

align 16
idt_start:
    times 512 dq 0 ; 256 64 bit entries
idt_end:

idtr: ; same as gdtr
    dw idt_end - idt_start - 1
    dq idt_start

section .bss
align 4096 ; moves address forward until divisible by 4096, these tables need to be on even address
PML4: ; contains pointer to tables
    resb 4096

align 4096
PDPT:
    resb 4096

align 4096
PD_start:
    resb 4096
PD_end:

align 16
stack_bottom:
    resb 8192
stack_top: