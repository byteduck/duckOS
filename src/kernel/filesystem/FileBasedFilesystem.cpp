#include <common/defines.h>
#include "FileBasedFilesystem.h"

FileBasedFilesystem::FileBasedFilesystem(DC::shared_ptr<FileDescriptor> file): Filesystem(file) {

}

bool FileBasedFilesystem::read_logical_blocks(size_t block, size_t count, uint8_t *buffer) {
	if(_file->seek(block * logical_block_size(), SEEK_SET) < 0) return false;
	return _file->read(buffer, count * logical_block_size()) >= 0;
}

size_t FileBasedFilesystem::logical_block_size() {
	return _logical_block_size;
}

bool FileBasedFilesystem::read_blocks(size_t block, size_t count, uint8_t *buffer) {
	if(_file->seek(block * block_size(), SEEK_SET) < 0) return false;
	return _file->read(buffer, count * block_size()) >= 0;
}
