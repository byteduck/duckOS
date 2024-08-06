ENTRY(start)

SECTIONS {
    /* This is a higher-half kernel, so it will live at 3GiB + 1MB in the virtual address space */

    . = 0xC0100000;
	PROVIDE(_KERNEL_TEXT = .);

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.multiboot*)
        *(.text*)
    }

    . = ALIGN(0x1000);
    PROVIDE(_KERNEL_TEXT_END = .);
    PROVIDE(_KERNEL_DATA = .);

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		start_ctors = .;
		*(.ctors)
		end_ctors = .;

		*(.rodata*)
	}

	.data : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data*)
	}

	.bss : AT(ADDR(.bss) - 0xC0000000)
	{
	    _PAGETABLES_START = .;
	    *(.pagetables*)
	    _PAGETABLES_END = .;
		*(COMMON)
		*(.bss*)

		. = ALIGN(4K);
		*(.kheap);
	}

	. = ALIGN(0x1000);
    PROVIDE(_KERNEL_DATA_END = .);
}