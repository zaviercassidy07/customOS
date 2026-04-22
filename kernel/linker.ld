ENTRY(start)

SECTIONS
{
    . = 0x10000;

    _kernel_start = .;

    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }

    _kernel_end = .;
}