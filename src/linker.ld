ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .text ALIGN(4K) :
    {
        KEEP(*(.multiboot))
        *(.text .text.*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata .rodata.*)
        *(.eh_frame .eh_frame.*)
    }

    .data ALIGN(4K) :
    {
        *(.data .data.*)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    . = ALIGN(4K);
    _kernel_end = .;
}