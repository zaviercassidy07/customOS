[org 0x8000]
[bits 16]

; load rest of kernel now as its easier with interupts
xor bx, bx
mov ah, 0x42
mov dl, [0x7D00]
mov ds, bx
mov si, dap
int 0x13 ; same as stage 1

jc diskError

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
    mov esp, 0x7C00   ; stack is now at bootloader, growing downwards. safe for tmp

    call initPaging

    jmp prepLong

; NOTE: This sets up access to 1MB of RAM
initPaging:
    mov edi, pt
    mov eax, 0x3

.fillPT:
    mov [edi], eax ; fill top half with data
    mov dword [edi + 4], 0 ; fill higher half with 0s

    add edi, 8
    add eax, 0x1000
    cmp edi, (pt + 4096) ; 4096 for 2MB access, change for less
    jne .fillPT

    ; Tables don't need to be zeroed as they are declared with db 0

    mov eax, pt
    or eax, 0x03
    mov [pd], eax
    mov dword [pd + 4], 0

    mov eax, pd
    or eax, 0x03
    mov [pdpt], eax
    mov dword [pdpt + 4], 0 ; again, populating table, fill half with 0s

    mov eax, pdpt
    or eax, 0x03
    mov [pml4], eax
    mov dword [pml4 + 4], 0

    mov eax, pml4
    or eax, 0x03
    mov [pml4 + 4080], eax
    mov dword [pml4 + 4084], 0

    mov eax, pdpt
    or eax, 0x03
    mov [pml4 + 4088], eax
    mov dword [pml4 + 4092], 0

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

    mov eax, pml4
    mov cr3, eax ; store address of pml4

    mov ebx, cr0
    or ebx, (1 << 31); enable paging again
    mov cr0, ebx

    jmp 0x18:long_entry

[bits 64]
long_entry:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax ; reinit registers

    mov rax, 0xFFFFFF8000010000 ; kernel has to be at this address
    jmp rax

[bits 16]
diskError:
    mov ah, 0x0E
    mov al, '!'
    int 0x10
    jmp $

dap:
    db 0x10 ; size, always 16b
    db 0 ; reserved
    dw 64 ; num sectors to read
    dw 0 ; offset in buffer to write to
    dw 0x1000 ; segment to write to
    dq 41 ; sector to start reading

[bits 32]
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

align 4096 ; moves address forward until divisible by 4096, these tables need to be on even address
pml4: ; contains pointer to tables
    times 4096 db 0

align 4096
pdpt:
    times 4096 db 0

align 4096
pd:
    times 4096 db 0

align 4096
pt:
    times 4096 db 0

times 20480 - ($ - $$) db 0 ; pad code