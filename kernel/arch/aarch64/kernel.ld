ENTRY(start)
KERNEL_BASE = 0xC000000000;

PHDRS
{
  text PT_LOAD ;
  data PT_LOAD ;
  bss PT_LOAD ;
}

SECTIONS {

    . = KERNEL_BASE + 0x80000;
	PROVIDE(_KERNEL_TEXT = .);

    .text : AT(ADDR(.text) - KERNEL_BASE)
    {
        *(.text.boot*)
        *(.text*)
    } :text

    . = ALIGN(0x1000);
    PROVIDE(_KERNEL_TEXT_END = .);
    PROVIDE(_KERNEL_DATA = .);

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_BASE)
	{
		start_ctors = .;
		*(.init_array)
		end_ctors = .;

		*(.rodata*)
	} :data
	
	.data : AT(ADDR(.data) - KERNEL_BASE)
	{
		*(.data*)
	} :data

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
	} :bss
	PROVIDE(_BSS_END = .);
	PROVIDE(_BSS_SIZE_LONG =( _BSS_END - _BSS_START) / 8);

	. = ALIGN(0x1000);
    PROVIDE(_KERNEL_DATA_END = .);
}