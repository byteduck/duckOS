ENTRY(start)

SECTIONS {
    /* This is a higher-half kernel, so it will live at 3GiB + 1MB in the virtual address space */

    . = 0xC0100000;
	PROVIDE(_KERNEL_START = .);

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.multiboot*)
        *(.text*)
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
	}
	
	PROVIDE(_KERNEL_END = .);
}