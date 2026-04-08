ENTRY(protected_mode_entry)

SECTIONS
{
    . = 0x9000;

    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}