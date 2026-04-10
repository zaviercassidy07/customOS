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
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax ; reinit registers

    lea rsp, stack_top ; lea is setting rsp to address of stack top, allows in place operations like adapt to 64 bit

    mov rax, 0x0F540F530F450F54 ; TEST encoded
    mov [0xB8000], rax ; print

    jmp $

gdt_start:
    ; bytes are 2 char, bytes in order are base-high, flags and limit-high, access perms, base mid, base mid, base low, limit mid, limit low
    dq 0x0000000000000000 ; null
    dq 0x00CF9A000000FFFF ; code 32bit (base=0, 4 GB)
    dq 0x00CF92000000FFFF ; data (base=0, 4 GB)
    dq 0x00A09A0000000000 ; code 64bit
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; length of gdt - 1
    dd gdt_start ; start address

align 4096 ; moves address forward until divisible by 4096, these tables need to be on even address
PML4: ; contains pointer to tables
    times 512 dq 0

align 4096
PDPT:
    times 512 dq 0

align 4096
PD_start:
    times 512 dq 0
PD_end:

section .bss
stack_bottom:
    resb 4096
stack_top: