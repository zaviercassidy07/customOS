ENTRY(start)

OFFSET = 0xFFFFFF8000000000;

SECTIONS
{
    . = OFFSET + 0x10000;

    _kernel_start = .;

    .text : AT(ADDR(.text) - OFFSET) { *(.text) }
    .data : AT(ADDR(.data) - OFFSET) { *(.data) }
    .bss : AT(ADDR(.bss) - OFFSET) { *(.bss) }

    _kernel_end = .;
}