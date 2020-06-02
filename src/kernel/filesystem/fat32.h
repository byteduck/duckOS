#ifndef FAT32_H
#define FAT32_H

typedef struct fat32part{
	uint8_t disk;
	uint16_t part_sect;
	uint8_t sectors_per_cluster;
	uint32_t root_dir_sect;
	uint32_t root_dir_clust;
	uint32_t cluster_begin_sect;
	uint8_t num_fats;
	uint32_t sectors_per_fat;
	uint16_t reserved_sectors;
	uint32_t fat_sect;
	uint32_t current_dir_clust;
} fat32part;

typedef struct fat32file{
	uint32_t cluster;
	uint32_t size;
	uint32_t dir_cluster;
	uint8_t attrib;
} fat32file;

bool isPartitionFAT32(int disk, int sect);
fat32part getFat32Part(int disk, int part_sect);
uint32_t clusterToLBA(uint32_t cluster);
uint32_t clusterToLBAOther(fat32part p, uint32_t cluster);
void setCurrentFat32part(fat32part p);
void listDir(uint32_t cluster, char *filter);
void listCurrentDir(char *filter);
uint8_t changeDir(char *dir);
bool isDirectory(fat32file file);
fat32file getFile(char *file);
bool exists(fat32file file);
static uint8_t changeOneDir(char *dir);
uint32_t getClusterOfEntry(uint8_t *entry);
uint32_t getClusterChainSize(uint32_t cluster);
uint32_t getFATSectorForCluster(uint32_t cluster);
uint32_t getNextCluster(uint32_t cluster);
uint32_t getClusterOfFile(char *file);
void printFileContents(fat32file f);
void printCurrentDir();
void executeFile(fat32file f);
fat32part getCurrentFat32Part();

#endif