#include "myfs.h"
#include <string.h>
#include <iostream>
#include <math.h>
#include <sstream>

const char *MyFs::MYFS_MAGIC = "MYFS";

MyFs::MyFs(BlockDeviceSimulator *blkdevsim_):blkdevsim(blkdevsim_) {
	myfs_header header;
	blkdevsim->read(0, sizeof(header), (char *)&header);

	if (strncmp(header.magic, MYFS_MAGIC, sizeof(header.magic)) != 0 ||
	    (header.version != CURR_VERSION)) {
		std::cout << "Did not find myfs instance on blkdev" << std::endl;
		std::cout << "Creating..." << std::endl;
		format();
		std::cout << "Finished!" << std::endl;
	}
}

void MyFs::format() {

    SuperBlock superblock;
	strncpy(superblock.header.magic, MYFS_MAGIC, sizeof(superblock.header.magic));
    superblock.header.version = CURR_VERSION;
    superblock.file_table_size = MAX_FILES * sizeof(FileEntry);
    superblock.free_block_bitmap_size = (MAX_BLOCKS + 7) / 8;
	blkdevsim->write(0, sizeof(superblock), (const char*)&superblock);
    char file_table[MAX_FILES * sizeof(FileEntry)] = {0};
    blkdevsim->write(sizeof(superblock), sizeof(file_table), file_table);
    char free_block_bitmap[(MAX_BLOCKS + 7) / 8] = {0};
    blkdevsim->write(sizeof(superblock) + sizeof(file_table), sizeof(free_block_bitmap), free_block_bitmap);
    char empty_block[BLOCK_SIZE] = {0};
    for (int i = 0; i < MAX_BLOCKS; i++) {
        blkdevsim->write(i * BLOCK_SIZE, BLOCK_SIZE, empty_block);
    }

    std::cout << "Formatted the file system." << std::endl;
}

void MyFs::create_file(std::string path_str, bool directory) {
	throw std::runtime_error("not implemented");
}

std::string MyFs::get_content(std::string path_str) {
	throw std::runtime_error("not implemented");
	return "";
}

void MyFs::set_content(std::string path_str, std::string content) {
	throw std::runtime_error("not implemented");
}

void list_dir(std::string path_str) {
	throw std::runtime_error("not implemented");
}

void MyFs::remove_file(std::string path_str) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].name == path_str) {
            file_index = i;
            break;
        }
    }
    if (file_index == -1) {
        throw std::runtime_error("File not found");
    }
    char free_block_bitmap[(MAX_BLOCKS + 7) / 8];
    blkdevsim->read(sizeof(SuperBlock) + sizeof(file_table), sizeof(free_block_bitmap), free_block_bitmap);
    int start_block = file_table[file_index].start_block;
    int num_blocks = ceil((double)file_table[file_index].size / BLOCK_SIZE);
    for (int i = 0; i < num_blocks; i++) {
        int block_index = start_block + i;
        free_block_bitmap[block_index / 8] &= ~(1 << (block_index % 8));
    }
    blkdevsim->write(sizeof(SuperBlock) + sizeof(file_table), sizeof(free_block_bitmap), free_block_bitmap);
    memset(&file_table[file_index], 0, sizeof(FileEntry));
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);
    std::cout << "File " << path_str << " removed." << std::endl;
}
