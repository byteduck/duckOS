ENTRY(start)

SECTIONS {
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