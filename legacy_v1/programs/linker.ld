ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x400000;    /* The exact address our Loader jumps to */

    .text : ALIGN(4K)
    {
        *(.text.start)  /* Put start.asm code FIRST. Critical! */
        *(.text)        /* Then C++ code */
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .bss : ALIGN(4K)
    {
        *(.bss)
    }
}
