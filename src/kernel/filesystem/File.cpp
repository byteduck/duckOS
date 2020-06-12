#include <kernel/kstdio.h>
#include "File.h"

File::File() {

}

File::~File() {

}

ResultRet<DC::shared_ptr<FileDescriptor>> File::open(DC::shared_ptr<File> file, int options) {
	auto ret = DC::make_shared<FileDescriptor>(file);
	ret->set_options(options);
	return ret;
}

bool File::is_inode() {
	return false;
}

ssize_t File::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return 0;
}

ssize_t File::write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) {
	return 0;
}


ssize_t File::read_dir_entry(FileDescriptor &fd, size_t offset, DirectoryEntry *buffer) {
	return 0;
}

