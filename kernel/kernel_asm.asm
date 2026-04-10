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

    mov word [0xB8000], 0x0F41

    jmp $

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

align 4096 ; moves address forward until divisible by 4096, these tables need to be on even address
page_directory: ; contains pointer to tables
    times 1024 dd 0

align 4096
page_table:
    times 1024 dd 0