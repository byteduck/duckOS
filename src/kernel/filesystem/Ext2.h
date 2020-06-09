#ifndef EXT2_H
#define EXT2_H

#include <kernel/filesystem/FileSystem.h>

//inode constants
#define ROOT_INODE 2
#define EXT2_SIGNATURE 0xEF53

//inode types
#define EXT2_FIFO 0x1000
#define EXT2_CHAR_DEVICE 0x2000
#define EXT2_DIRECTORY 0x4000
#define EXT2_BLOCK_DEVICE 0x6000
#define EXT2_FILE 0x8000
#define EXT2_SYMLINK 0xA000
#define EXT2_SOCKET 0xC000

//inode flags
#define EXT2_SYNCHRONOUS 0x8
#define EXT2_IMMUTABLE 0x10
#define EXT2_APPEND_ONLY 0x20
#define EXT2_DUMP_EXCLUDE 0x40
#define EXT2_JOURNAL_FILE 0x40000

typedef struct __attribute__((packed)) ext2_superblock{
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t superuser_blocks;
	uint32_t unallocated_blocks;
	uint32_t unallocated_inodes;
	uint32_t superblock_block;
	uint32_t block_size; //do 1024 << block_size to get the block size
	uint32_t fragment_size; //Do 1024 << fragment_size to get the fragment size
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount;
	uint32_t last_write;
	uint16_t times_mounted; //Since last fsck
	uint16_t mounts_allowed; //fsck must be done after this amount surpassed
	uint16_t signature;
	uint16_t state;
	uint16_t error_action;
	uint16_t version_minor;
	uint32_t last_check; //POSIX time of last fsck
	uint32_t check_interval; //POSIX time between fscks
	uint32_t os_id;
	uint32_t version_major;
	uint16_t reserved_user;
	uint16_t reserved_group;
	//Start extended fields
	uint32_t first_inode;
	uint16_t inode_size;
	uint16_t superblock_group;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t ro_features;
	uint8_t filesystem_id[16];
	uint8_t volume_name[16];
	uint8_t last_mount_path[64];
	uint32_t compression;
	uint8_t file_prealloc_blocks;
	uint8_t directory_prealloc_blocks;
	uint16_t UNUSED_AARON_IS_THE_BEST;
	uint8_t journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t orphan_inode_head;
	uint8_t extra[276];
} ext2_superblock;

typedef struct __attribute__((packed)) ext2_block_group_descriptor{
	uint32_t block_usage_bitmap;
	uint32_t inode_usage_bitmap;
	uint32_t inode_table;
	uint16_t unallocated_blocks;
	uint16_t allocated_blocks;
	uint16_t num_directories;
	uint8_t unused[14];
} ext2_block_group_descriptor;

typedef struct __attribute__((packed)) ext2_directory{
	uint32_t inode;
	uint16_t size;
	uint8_t name_length;
	uint8_t type;
} ext2_directory;

class Ext2Filesystem;

class Ext2Inode: public Inode {
public:

	typedef struct __attribute__((packed)) Raw {
		uint16_t type;
		uint16_t user_id;
		uint32_t size_lower;
		uint32_t last_access_time;
		uint32_t creation_time;
		uint32_t last_modification_time;
		uint32_t deletion_time;
		uint16_t group_id;
		uint16_t hard_links; //Hard links to this node
		uint32_t sectors_in_use; //Hard disk sectors, not ext2 blocks.
		uint32_t flags;
		uint32_t os_specific_1;
		uint32_t block_pointers[12];
		uint32_t s_pointer;
		uint32_t d_pointer;
		uint32_t t_pointer;
		uint32_t generation_number;
		uint32_t extended_attribute_block;
		uint32_t size_upper;
		uint32_t block_address;
		uint32_t os_specific_2[3];
	} Raw;

	Raw raw;

	Ext2Inode(Ext2Filesystem& filesystem, InodeID i);
	uint32_t get_block_group();
	uint32_t get_index();
	uint32_t get_block();
	bool is_directory() override;
	bool is_link() override;
	void read_raw();
	bool read(uint32_t start, uint32_t length, uint8_t* buf) override;
	Inode* find_rawptr(DC::string name) override;
	Ext2Filesystem& ext2fs();
};

class Ext2Filesystem: public Filesystem {
public:
	ext2_superblock superblock;
	uint32_t block_size;
	uint32_t block_group_descriptor_table;
	uint32_t blocks_per_inode_table;
	uint32_t sectors_per_inode_table;
	uint32_t sectors_per_block;
	uint32_t num_block_groups;
	uint32_t inodes_per_block;

	Ext2Filesystem(BlockDevice& device);
	const char* name() const;
	static bool probe(BlockDevice* dev);
	Inode * get_inode_rawptr(InodeID id) override;
	void get_superblock(ext2_superblock *sb);
	uint32_t block_to_sector(uint32_t block);
	void read_slink(uint32_t block, uint8_t *buf);
	void read_dlink(uint32_t block, uint8_t *buf);
	bool read_block(uint32_t block, uint8_t *buf);
};

#endif
