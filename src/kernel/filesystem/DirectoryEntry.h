#ifndef DUCKOS_DIRECTORYENTRY_H
#define DUCKOS_DIRECTORYENTRY_H

#include "InodeMetadata.h"

#define NAME_MAXLEN 256
#define TYPE_UNKNOWN 0
#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_CHARACTER_DEVICE 3
#define TYPE_BLOCK_DEVICE 4
#define TYPE_FIFO 5
#define TYPE_SOCKET 6
#define TYPE_SYMLINK 7

class DirectoryEntry {
public:
	InodeID id;
	size_t name_length;
	uint8_t type;
	char name[NAME_MAXLEN];
};


#endif //DUCKOS_DIRECTORYENTRY_H
