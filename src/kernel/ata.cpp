#include <common.h>
#include <ata.h>

/*
* This system is to read disks formatted with an MBR only.
* For reading specific filesystems, see the respective files in c/drivers/disk.
*/
uint8_t boot_disk = 0;
uint8_t ata_buf[512];
uint8_t ata_buf2[512];

void prepareDisk(int disk, int address){
	outb(0x1F1,0x00);
	outb(0x1F2,0x01);
	outb(0x1F3,(uint8_t)address);
	outb(0x1F4,(uint8_t)(address >> 8));
	outb(0x1F5,(uint8_t)(address >> 16));
	outb(0x1F6,0xE0 | (disk << 4) | ((address >> 24) & 0x0F));
	outb(0x1F7,0x20);
	while (!(inb(0x1F7) & 0x08)) {}
}

void readSector(int disk, int address, uint8_t *sect){
	prepareDisk(disk, address);
	for(int i = 0; i < 256; i++){
		uint16_t tmp = inw(0x1F0);
		sect[i*2] = (uint8_t)tmp;
		sect[i*2+1] = (uint8_t)(tmp >> 8);
	}
}

void readSectors(int disk, int address, int sectors, uint8_t *sect){
	for(int i = 0; i < sectors; i++){
		readSector(disk,address+i,sect+i*512);
	}
}

int getFirstPartition(int disk){
	prepareDisk(disk,0);
	uint16_t pos = 0;
	for(int i = 0; i < 256; i++){
		uint16_t tmpword = inw(0x1F0);
		if(i == 227){
			pos = tmpword;
		}
	}
	return pos;
}

int getFSType(int disk){

}
