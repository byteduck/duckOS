#include <kernel/kstddef.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/kstdio.h>
#include <kernel/filesystem/Ext2.h>
#include <kernel/filesystem/FileSystem.h>
#include <common/cstring.h>
#include <common/defines.h>

extern uint8_t ata_buf[512], ata_buf2[512];

Ext2Filesystem::Ext2Filesystem(DC::shared_ptr<FileDescriptor> file) : FileBasedFilesystem(file) {
	_fsid = EXT2_FSID;
}

void Ext2Filesystem::init() {
	read_superblock(&superblock);
	set_block_size(1024u << (superblock.block_size));
	block_group_descriptor_table = superblock.superblock_block+1;
	blocks_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/block_size();
	sectors_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/512;
	sectors_per_block = block_size()/512;
	num_block_groups = superblock.total_blocks/superblock.blocks_per_group + (superblock.total_blocks % superblock.blocks_per_group != 0);
	inodes_per_block = block_size()/superblock.inode_size;
}

bool Ext2Filesystem::probe(FileDescriptor& file){
	file.seek(512 * 2, SEEK_SET); //Supercluster begins at partition sector + 2
	file.read(ata_buf, 512);
	return ((ext2_superblock *)ata_buf)->signature == EXT2_SIGNATURE;
}

void Ext2Filesystem::read_superblock(ext2_superblock *sb){
	read_logical_blocks(2, 2, (uint8_t*)sb);
	if(sb->version_major < 1){ //If major version is less than 1, then use defaults for stuff
		sb->first_inode = 11;
		sb->inode_size = 128;
	}
}

void Ext2Filesystem::read_slink(uint32_t block, uint8_t *buf){
	uint8_t *bbuf = static_cast<uint8_t *>(kmalloc(block_size()));
	read_blocks(block, 1, bbuf);
	uint32_t *blocks = (uint32_t *)bbuf;
	uint32_t numblocks = block_size()/sizeof(uint32_t);
	for(int i = 0; i < numblocks; i++){
		if(blocks[i] == 0) break;
		read_blocks(blocks[i], 1, buf+i*block_size());
	}
	kfree(bbuf);
}

void Ext2Filesystem::read_dlink(uint32_t block, uint8_t *buf){
	uint8_t *bbuf = static_cast<uint8_t *>(kmalloc(block_size()));
	read_blocks(block, 1, bbuf);
	uint32_t *blocks = (uint32_t *)bbuf;
	uint32_t numblocks = block_size()/sizeof(uint32_t);
	uint32_t singsize = numblocks*block_size();
	for(int i = 0; i < numblocks; i++){
		if(blocks[i] == 0) break;
		read_blocks(blocks[i], 1, buf+i*singsize);
	}
	kfree(bbuf);
}

Inode* Ext2Filesystem::get_inode_rawptr(InodeID id) {
	auto ret = new Ext2Inode(*this, id);
	((Ext2Inode*)ret)->read_raw();
	return static_cast<Inode *>(ret);
}

char *Ext2Filesystem::name() {
	return "EXT2";
}

InodeID Ext2Filesystem::root_inode() {
	return 2;
}

////// INODE

Ext2Inode::Ext2Inode(Ext2Filesystem& filesystem, InodeID id): Inode(filesystem, id) {

}

uint32_t Ext2Inode::get_block_group(){
	return (id - 1) / ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_index() {
	return (id - 1) % ext2fs().superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_block(){
	return (get_index() * ext2fs().superblock.inode_size) / fs.block_size();
}

void Ext2Inode::read_raw() {
	auto* block_buf = new uint8_t[fs.block_size()];

	uint32_t bg = get_block_group();
	ext2fs().read_blocks(2, 1, block_buf);
	auto* d = (ext2_block_group_descriptor*) block_buf;
	for(int i = 0; i < bg; i++) d++; //note to self - d++ adds to the pointer by sizeof(ext2_block_group_descriptor)
	uint32_t inode_table = d->inode_table;

	ext2fs().read_blocks(inode_table + get_block(), 1, block_buf);
	uint32_t index = get_index() % ext2fs().superblock.inodes_per_group;
	Raw* inodeRaw = reinterpret_cast<Raw *>(block_buf);
	for(int i = 0; i < index; i++) inodeRaw++; //same here as above

	memcpy(&raw, inodeRaw, sizeof(Ext2Inode::Raw));
	delete[] block_buf;

	InodeMetadata meta;
	meta.mode = raw.mode;
	meta.size = raw.size;
	_metadata = meta;
}

size_t Ext2Inode::read(uint32_t start, uint32_t length, uint8_t *buf) {
	ASSERT(start >= 0);
	if(raw.size == 0) return 0;
	//TODO: symlinks
	size_t first_block = start / fs.block_size();
	size_t first_block_start = start % fs.block_size();
	size_t last_block = (start + length) / fs.block_size();
	size_t last_block_end = length % fs.block_size();
	if(last_block >= num_blocks()) {
		last_block = num_blocks() - 1;
	}

	auto block_buf = new uint8_t[fs.block_size()];
	for(auto i = first_block; i <= last_block; i++) {
		uint32_t block = 0;
		if(i < 12) { //Direct block pointer
			block = raw.block_pointers[i];
		} else if(i < fs.block_size() / sizeof(uint32_t) + 12) { //Singly indirect block pointer
			ext2fs().read_blocks(raw.s_pointer, 1, block_buf);
			block = ((uint32_t*)block_buf)[i - 12];
		} //TODO: doubly indirect/triply indirect
		ext2fs().read_blocks(block, 1, block_buf);
		if(i == first_block) {
			memcpy(buf, block_buf + first_block_start, fs.block_size() - first_block_start);
		} else if(i == last_block) {
			memcpy(buf + (i - first_block) * fs.block_size(), block_buf, last_block_end);
		} else {
			memcpy(buf + last_block * fs.block_size(), block_buf, fs.block_size());
		}
	}
	delete[] block_buf;
	return true;
}

Inode *Ext2Inode::find_rawptr(DC::string find_name) {
	Inode *ret = nullptr;
	if((raw.mode & 0xF000u) == EXT2_DIRECTORY) {
		auto* buf = static_cast<uint8_t *>(kmalloc(fs.block_size()));
		for(int i = 0; i < 12; i++) {
			uint32_t block = raw.block_pointers[i];
			if(block == 0 || block > ext2fs().superblock.total_blocks);
			ext2fs().read_blocks(block, 1, buf);
			auto* dir = reinterpret_cast<ext2_directory *>(buf);
			uint32_t add = 0;
			char name_buf[257];
			while(dir->inode != 0 && add < fs.block_size()) {
				memcpy(name_buf, &dir->type+1, dir->name_length);
				name_buf[dir->name_length] = '\0';
				if(find_name == name_buf){
					ret = fs.get_inode_rawptr(dir->inode);
				}
				add += dir->size;
				dir = (ext2_directory*)((size_t)dir + dir->size);
			}
		}
		kfree(buf);
	}
	return ret;
}

Ext2Filesystem& Ext2Inode::ext2fs() {
	return (Ext2Filesystem &)(fs);
}

size_t Ext2Inode::num_blocks() {
	return (raw.size + fs.block_size() - 1) / fs.block_size();
}
