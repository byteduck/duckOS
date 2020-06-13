#include "InodeMetadata.h"

bool InodeMetadata::is_directory() const {
	return (mode & 0xF000u) == MODE_DIRECTORY;
}

bool InodeMetadata::is_simple_file() const {
	return (mode & 0xF000u) == MODE_FILE;
}

bool InodeMetadata::exists() const {
	return inode_id != 0;
}

bool InodeMetadata::is_block_device() const {
	return (mode & 0xF000u) == MODE_BLOCK_DEVICE;
}

bool InodeMetadata::is_character_device() const {
	return (mode & 0xF000u) == MODE_CHAR_DEVICE;
}

bool InodeMetadata::is_device() const {
	return (mode & 0xF000u) == MODE_CHAR_DEVICE ||  (mode & 0xF000u) == MODE_BLOCK_DEVICE;
}
