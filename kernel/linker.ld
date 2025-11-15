ENTRY(_start)

KERNEL_VMA = 0xffffffff80000000;

SECTIONS
{
    . = KERNEL_VMA;

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss*)
    }
}
