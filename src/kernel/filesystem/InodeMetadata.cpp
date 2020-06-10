#include "InodeMetadata.h"

bool InodeMetadata::is_directory() {
	return (mode & 0xF000u) == MODE_DIRECTORY;
}

bool InodeMetadata::is_simple_file() {
	return (mode & 0xF000u) == MODE_FILE;
}

bool InodeMetadata::exists() {
	return inode_id != 0;
}
