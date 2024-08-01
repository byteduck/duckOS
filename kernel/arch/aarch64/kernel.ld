ENTRY(start)
KERNEL_BASE = 0xFFFF000000000000;

SECTIONS {

    . = KERNEL_BASE + 0x80000;
	PROVIDE(_KERNEL_TEXT = .);

    .text : AT(ADDR(.text) - KERNEL_BASE)
    {
        *(.text.boot*)
        *(.text*)
    }

    . = ALIGN(0x1000);
    PROVIDE(_KERNEL_TEXT_END = .);
    PROVIDE(_KERNEL_DATA = .);

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_BASE)
	{
		start_ctors = .;
		*(.init_array)
		end_ctors = .;

		*(.rodata*)
	}
	
	.data : AT(ADDR(.data) - KERNEL_BASE)
	{
		*(.data*)
	}

	PROVIDE(_BSS_START = .);
	.bss : AT(ADDR(.bss) - KERNEL_BASE)
	{
	    _PAGETABLES_START = .;
	    *(.pagetables*)
	    _PAGETABLES_END = .;
		*(COMMON)
		*(.bss*)

		. = ALIGN(4K);
		*(.kheap);

		. = ALIGN(4K);
		*(.meminit*);
	}
	PROVIDE(_BSS_END = .);
	PROVIDE(_BSS_SIZE_LONG =( _BSS_END - _BSS_START) / 8);

	. = ALIGN(0x1000);
    PROVIDE(_KERNEL_DATA_END = .);
}