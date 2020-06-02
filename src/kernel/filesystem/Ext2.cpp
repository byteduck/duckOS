#include <common.h>
#include <memory/heap.h>
#include <stdio.h>
#include <filesystem/Ext2.h>
#include <filesystem/FileSystem.h>

extern uint8_t ata_buf[512], ata_buf2[512];

Ext2Filesystem::Ext2Filesystem(BlockDevice *device) : Filesystem(device) {
	get_superblock(&superblock);
	block_size = 1024 << (superblock.block_size);
	block_group_descriptor_table = superblock.superblock_block+1;
	blocks_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/block_size;
	sectors_per_inode_table = (superblock.inode_size * superblock.inodes_per_group)/512;
	sectors_per_block = block_size/512;
	num_block_groups = superblock.total_blocks/superblock.blocks_per_group + (superblock.total_blocks % superblock.blocks_per_group != 0);
	inodes_per_block = block_size/superblock.inode_size;
}

bool Ext2Filesystem::probe(BlockDevice* dev){
	dev->read_block(2, ata_buf); //Supercluster begins at partition sector + 2
	return ((ext2_superblock *)ata_buf)->signature == EXT2_SIGNATURE;
}

void Ext2Filesystem::get_superblock(ext2_superblock *sb){
	device->read_block(2, (uint8_t *)sb);
	if(sb->version_major < 1){ //If major version is less than 1, then use defaults for stuff
		sb->first_inode = 11;
		sb->inode_size = 128;
	}
}

bool Ext2Filesystem::read_block(uint32_t block, uint8_t *buf){
	return device->read_blocks(block_to_sector(block), sectors_per_block, buf);
}

void Ext2Filesystem::read_slink(uint32_t block, uint8_t *buf){
	uint8_t *bbuf = static_cast<uint8_t *>(kmalloc(block_size));
	read_block(block, bbuf);
	uint32_t *blocks = (uint32_t *)bbuf;
	uint32_t numblocks = block_size/sizeof(uint32_t);
	for(int i = 0; i < numblocks; i++){
		if(blocks[i] == 0) break;
		read_block(blocks[i], buf+i*block_size);
	}
	kfree(bbuf, block_size);
}

void Ext2Filesystem::read_dlink(uint32_t block, uint8_t *buf){
	uint8_t *bbuf = static_cast<uint8_t *>(kmalloc(block_size));
	read_block(block, bbuf);
	uint32_t *blocks = (uint32_t *)bbuf;
	uint32_t numblocks = block_size/sizeof(uint32_t);
	uint32_t singsize = numblocks*block_size;
	for(int i = 0; i < numblocks; i++){
		if(blocks[i] == 0) break;
		read_block(blocks[i], buf+i*singsize);
	}
	kfree(bbuf, block_size);
}

uint32_t Ext2Filesystem::block_to_sector(uint32_t block){
	return (block_size/512)*block;
}

void Ext2Filesystem::get_inode(InodeID id, Inode* inode) {
	inode->id = id;
	inode->fs = this;
	((Ext2Inode*)inode)->read_raw();
}

const char *Ext2Filesystem::name() const {
	return "EXT2";
}

////// INODE

uint32_t Ext2Inode::get_block_group(){
	return (id - 1) / fs->superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_index() {
	return (id - 1) % fs->superblock.inodes_per_group;
}

uint32_t Ext2Inode::get_block(){
	return (get_index() * fs->superblock.inode_size) / fs->block_size;
}

void Ext2Inode::read_raw() {
	uint8_t* block_buf = static_cast<uint8_t *>(kmalloc(fs->sectors_per_block * 512));

	uint32_t bg = get_block_group();
	ext2_block_group_descriptor *d = (ext2_block_group_descriptor *) fs->read_block(2, block_buf);
	for(int i = 0; i < bg; i++) d++; //note to self - d++ adds to the pointer by sizeof(ext2_block_group_descriptor)
	uint32_t inode_table = d->inode_table;

	//TODO: Is this safe? Idk, I wrote this a long time ago.
	fs->read_block(inode_table + get_block(), block_buf);
	uint32_t index = get_index() % fs->inodes_per_block;
	Raw* inodeRaw = reinterpret_cast<Raw *>(block_buf);
	for(int i = 0; i < index; i++) inodeRaw++; //same here as above

	memcpy(&raw, inodeRaw, sizeof(Ext2Inode::Raw));
	kfree(block_buf, fs->sectors_per_block * 512);
}

bool Ext2Inode::read(uint32_t start, uint32_t length, uint8_t *buf) {
	//TODO ignores start/length, only reads whole thing
	for(int i = 0; i < 12; i++){
		uint32_t block = raw.block_pointers[i];
		if(block == 0 || block > fs->superblock.total_blocks){break;}
		fs->read_block(block, buf);
	}
	if(raw.s_pointer)
		fs->read_slink(raw.s_pointer, buf+12*fs->block_size);
	if(raw.d_pointer)
		fs->read_dlink(raw.d_pointer, buf+(fs->block_size/sizeof(uint32_t))*fs->block_size+12*fs->block_size);
	if(raw.t_pointer)
		printf("WARNING! File uses t_pointer. Will not work.\n");
	return true;
}
