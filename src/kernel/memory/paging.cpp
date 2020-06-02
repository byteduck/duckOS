#include <common.h>
#include <stdio.h>
#include <memory/paging.h>
#include <interrupt/isr.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t exec_page_table[1024] __attribute__((aligned(4096)));
#define krnlstartPhys = &kernelstart-HIGHER_HALF;
#define krnlendPhys = &krnlend-HIGHER_HALF;
extern uint32_t BootPageDirectory;

void setupPaging(){
	uint32_t i,j;
	page_directory[0]= 0x83;
	for(i = 1; i < (HIGHER_HALF >> 22); i++){
		page_directory[i] = 0x0;
	}
	page_directory[i] = 0x83;
	i++;
	for(i=i; i < 1024; i++){
		page_directory[i] = 0;
	}
	uint32_t *d = (uint32_t*)0x1000;
	load_page_dir((uint32_t *)((uint32_t)&page_directory[0]-HIGHER_HALF));
}

void exec(uint8_t *prog){
	exec_page_table[0] = ((uint32_t)&prog[0]-HIGHER_HALF) | 0x3;
	page_directory[0] = ((uint32_t)&exec_page_table[0]-HIGHER_HALF) | 0x3;
	load_page_dir((uint32_t *)((uint32_t)&page_directory[0]-HIGHER_HALF));
	((void(*)())0)();
	exec_page_table[0] = 0x83;
	load_page_dir((uint32_t *)((uint32_t)&page_directory[0]-HIGHER_HALF));
}

void pageFaultHandler(struct registers *r){
	cli();
	//uint32_t err_pos;
	//asm("mov %0, %%cr2" : "=r" (err_pos));
	bool other = false;
	switch(r->err_code){
		case 0:
		case 1:
			PANIC("KRNL_READ_NONPAGED_AREA", "", false);
			break;
		case 2:
		case 3:
			PANIC("KRNL_WRITE_NONPAGED_AREA", "", false);
			break;
		case 4:
		case 5:
			PANIC("USR_READ_NONPAGED_AREA", "", false);
			break;
		case 6:
		case 7:
			PANIC("USR_WRITE_NONPAGED_AREA", "", false);
			break;
		default:
			PANIC("UNKNOWN_PAGE_FAULT", "", false);
			other = true;
			break;
	}

	//printf("At 0x%X\n\n",err_pos);
	print_regs(r);
	while(true);
}
