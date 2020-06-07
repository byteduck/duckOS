ENTRY(start)

SECTIONS {
    /* This is a higher-half kernel, so it will live at 3GiB + 1MB in the virtual address space */

    . = 0xC0100000;
	krnlstart = .;

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text*)
    }
	
	.data : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data*)
	}
	
	.bss : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss*)
	}
	
	krnlend = .;
}