#ifndef ATA_H
#define ATA_H

void readSector(int disk, int address, uint8_t *sect);
void prepareDisk(int disk, int address);
void readSectors(int disk, int address, int sectors, uint8_t *sect);
int getFSType(int disk);
int getFirstPartition(int disk);

#endif
