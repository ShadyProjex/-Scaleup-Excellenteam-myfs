#include "myfs.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cmath>

const char *MyFs::MYFS_MAGIC = "MYFS";

MyFs::MyFs(BlockDeviceSimulator *blkdevsim_) : blkdevsim(blkdevsim_) {
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
void MyFs::create_root_directory() {
    FileEntry root_entry;
    strncpy(root_entry.name, "/", sizeof(root_entry.name));
    root_entry.name[sizeof(root_entry.name) - 1] = '\0';
    root_entry.is_directory = true;
    root_entry.size = 0;
    root_entry.start_block = 0; // Assuming root directory doesn't need blocks initially
    root_entry.parent_index = -1; // Indicating it's the root directory
    root_entry.num_children = 0;

    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);

    // Find a free entry in the file table
    int free_index = find_free_file_entry(file_table);
    if (free_index == -1) {
        throw std::runtime_error("No space left for new files");
    }

    // Update file table with root directory entry
    file_table[free_index] = root_entry;
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);
}

void MyFs::format() {
    SuperBlock superblock;
    strncpy(superblock.header.magic, MYFS_MAGIC, sizeof(superblock.header.magic));
    superblock.header.version = CURR_VERSION;
    superblock.file_table_size = MAX_FILES * sizeof(FileEntry);
    superblock.free_block_bitmap_size = (MAX_BLOCKS + 7) / 8;
    blkdevsim->write(0, sizeof(superblock), (const char*)&superblock);
    clear_file_table();
    clear_free_block_bitmap(superblock);
    clear_data_blocks();
    create_root_directory();
    std::cout << "Formatted the file system." << std::endl;
}

void MyFs::create_file(std::string path_str, bool directory) {
    // Extract parent directory path
    size_t last_slash = path_str.find_last_of('/');
    std::string parent_path = last_slash == std::string::npos ? "/" : path_str.substr(0, last_slash);
    std::string file_name = last_slash == std::string::npos ? path_str : path_str.substr(last_slash + 1);
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    // Find the parent directory entry
    int parent_index = find_file_entry(parent_path, file_table);
    if (parent_index == -1 && !parent_path.empty()) {
        throw std::runtime_error("Parent directory does not exist");
    }
    // Check if the file already exists
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strcmp(file_table[i].name, file_name.c_str()) == 0 && file_table[i].parent_index == parent_index) {
            throw std::runtime_error("File or directory already exists");
        }
    }
    // Find a free entry in the file table
    int free_index = find_free_file_entry(file_table);
    if (free_index == -1) {
        throw std::runtime_error("No space left for new files");
    }

    // Initialize the new file entry
    FileEntry new_file;
    strncpy(new_file.name, file_name.c_str(), sizeof(new_file.name));
    new_file.name[sizeof(new_file.name) - 1] = '\0';
    new_file.is_directory = directory;
    new_file.size = 0;
    new_file.parent_index = parent_index;
    new_file.num_children = 0;
    // Allocate blocks for the file if it's not a directory
    if (!directory) {
        int start_block = allocate_blocks(1);
        new_file.start_block = start_block;
    } else {
        new_file.start_block = 0; // Directories don't need blocks initially
    }
    // Update file table entry
    file_table[free_index] = new_file;
    // Update parent directory's child list
    if (parent_index != -1) {
        file_table[parent_index].child_indices[file_table[parent_index].num_children] = free_index;
        file_table[parent_index].num_children++;
    }
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);

    if (directory) {
        // Append '/' to directory name
        strncat(new_file.name, "/", sizeof(new_file.name) - strlen(new_file.name) - 1);
    }

    std::cout << (directory ? "Directory" : "File") << " " << path_str << " created." << std::endl;
}

std::string MyFs::get_content(std::string path_str) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    int file_index = find_file_entry(path_str, file_table);
    if (file_index == -1 || file_table[file_index].is_directory) {
        throw std::runtime_error("File not found or is a directory");
    }
    std::string content;
    read_file_content(file_table[file_index], content,BLOCK_SIZE);
    return content;
}

void MyFs::set_content(std::string path_str, std::string content) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    int file_index = find_file_entry(path_str, file_table);
    if (file_index == -1 || file_table[file_index].is_directory) {
        throw std::runtime_error("File not found or is a directory");
    }
    int start_block=allocate_and_write_blocks(file_table[file_index], content,BLOCK_SIZE);
    update_file_entry(file_table[file_index], content.size(),start_block);
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);
}

void MyFs::list_dir(std::string path_str) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);

    // Find the directory entry
    int dir_index = find_file_entry(path_str, file_table);
    if (dir_index == -1 || !file_table[dir_index].is_directory) {
        throw std::runtime_error("Directory not found");
    }

    // List directory contents
    for (int i = 0; i < file_table[dir_index].num_children; ++i) {
        int child_index = file_table[dir_index].child_indices[i];
        std::cout << file_table[child_index].name <<(file_table[child_index].is_directory?"/":"") << " " << file_table[child_index].size << std::endl;
    }
}


void MyFs::remove_file(std::string path_str) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    // Find the file entry
    int file_index = find_file_entry(path_str, file_table);
    if (file_index == -1) {
        throw std::runtime_error("File not found");
    }
    free_allocated_blocks(file_table[file_index]);
    clear_file_entry(file_table[file_index]);
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);
    std::cout << "File " << path_str << " removed." << std::endl;
}


int MyFs::find_free_file_entry(FileEntry file_table[]) {
    for (int i = 0; i < MAX_FILES; ++i) {
        if (file_table[i].name[0] == '\0') {
            return i;
        }
    }
    return -1;
}

int MyFs::find_file_entry(std::string path_str, FileEntry file_table[]) {
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strcmp(file_table[i].name, path_str.c_str()) == 0) {
            return i;
        }
    }
    return -1;
}

int MyFs::find_directory_entry(std::string path_str, FileEntry file_table[]) {
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strcmp(file_table[i].name, path_str.c_str()) == 0 && file_table[i].is_directory) {
            return i;
        }
    }
    return -1;
}

void MyFs::read_file_content(const FileEntry& file_entry, std::string& content,const int blocksize) {
    int current_block = file_entry.start_block;
    int remaining_size = file_entry.size;
    std::cout<<" start "<<current_block<<" ,num blocks"<<remaining_size<<"\n";
    while (remaining_size > 0) {
        char block[blocksize];
        int read_size = std::min(blocksize, remaining_size);
        blkdevsim->read(current_block * blocksize, read_size, block);
        std::cout<<" block  "<<block<<"\n";
        content.append(block, read_size);
        remaining_size -= read_size;
        current_block++;
    }
}

int MyFs::allocate_and_write_blocks(const FileEntry& file_entry, const std::string& content,const int blocksize) {
    int num_blocks = ceil((double)content.size() / blocksize);
    int start_block =allocate_blocks(num_blocks);
    std::cout<<"ALLOCATE TEST content: "<<content<<" start "<<start_block<<" ,num blocks"<<num_blocks<<"\n";
    int current_block = start_block;
    int remaining_size = content.size();
    int offset = 0;
    while (remaining_size > 0) {
        int write_size = std::min(blocksize, remaining_size);
        blkdevsim->write(current_block * blocksize, write_size, content.substr(offset, write_size).c_str());
        remaining_size -= write_size;
        offset += write_size;
        current_block++;
    }
    return start_block;
}

void MyFs::update_file_entry(FileEntry& file_entry, int content_size,const int start_block) {
    file_entry.size = content_size;
    file_entry.is_directory= false;
    file_entry.start_block=start_block;
}

void MyFs::free_allocated_blocks(const FileEntry& file_entry) {
    SuperBlock superblock;
    blkdevsim->read(0, sizeof(superblock), (char*)&superblock);

    char free_block_bitmap[(MAX_BLOCKS + 7) / 8];
    blkdevsim->read(sizeof(SuperBlock) + superblock.file_table_size, sizeof(free_block_bitmap), free_block_bitmap);

    int start_block = file_entry.start_block;
    int num_blocks = ceil((double)file_entry.size / BLOCK_SIZE);

    for (int i = 0; i < num_blocks; ++i) {
        free_block_bitmap[(start_block + i) / 8] &= ~(1 << ((start_block + i) % 8));
    }

    blkdevsim->write(sizeof(SuperBlock) + superblock.file_table_size, sizeof(free_block_bitmap), free_block_bitmap);
}

void MyFs::clear_file_entry(FileEntry& file_entry) {
    memset(&file_entry, 0, sizeof(FileEntry));
}

void MyFs::clear_file_table() {
    FileEntry file_table[MAX_FILES] = {0};
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)&file_table);
}

void MyFs::clear_free_block_bitmap(SuperBlock superblock) {
    char free_block_bitmap[(MAX_BLOCKS + 7) / 8] = {0};
    blkdevsim->write(sizeof(superblock) + superblock.file_table_size, sizeof(free_block_bitmap), free_block_bitmap);
}

void MyFs::clear_data_blocks() {
    char empty_block[BLOCK_SIZE] = {0};
    for (int i = 0; i < MAX_BLOCKS; ++i) {
        blkdevsim->write(i * BLOCK_SIZE, BLOCK_SIZE, empty_block);
    }
}

int MyFs::allocate_blocks(int num_blocks) {
    SuperBlock superblock;
    blkdevsim->read(0, sizeof(superblock), (char*)&superblock);
    char free_block_bitmap[(MAX_BLOCKS + 7) / 8];
    blkdevsim->read(sizeof(SuperBlock) + superblock.file_table_size, sizeof(free_block_bitmap), free_block_bitmap);
    // Find free blocks
    int start_block = -1;
    int free_blocks_found = 0;
    for (int i = 0; i < MAX_BLOCKS; ++i) {
        if ((free_block_bitmap[i / 8] & (1 << (i % 8))) == 0) {
            // Found a free block
            if (start_block == -1) {
                start_block = i;
            }
            free_blocks_found++;
            if (free_blocks_found == num_blocks) {
                break;
            }
        } else {
            // Reset if a block is not free
            start_block = -1;
            free_blocks_found = 0;
        }
    }
    if (free_blocks_found != num_blocks) {
        throw std::runtime_error("Not enough free blocks available");
    }
    // Mark blocks as allocated
    for (int i = 0; i < num_blocks; ++i) {
        free_block_bitmap[(start_block + i) / 8] |= (1 << ((start_block + i) % 8));
    }
    // Write back the updated bitmap
    blkdevsim->write(sizeof(SuperBlock) + superblock.file_table_size, sizeof(free_block_bitmap), free_block_bitmap);
    return start_block;
}

//bonus:


void MyFs::remove_directory(std::string dir_path) {
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);
    int dir_index = find_directory_entry(dir_path, file_table);
    if (dir_index == -1) {
        throw std::runtime_error("Directory not found");
    }
    // Clear the directory entry in the file table
    memset(&file_table[dir_index], 0, sizeof(FileEntry));
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);

    std::cout << "Directory " << dir_path << " removed." << std::endl;
}

void MyFs::move_file(std::string old_path, std::string new_path) {
    if (old_path.empty() || old_path[0] != '/' || new_path.empty() || new_path[0] != '/') {
        throw std::runtime_error("Invalid paths");
    }
    // Read the file table
    FileEntry file_table[MAX_FILES];
    blkdevsim->read(sizeof(SuperBlock), sizeof(file_table), (char*)file_table);

    // Find the file or directory entry
    int file_index = find_file_entry(old_path, file_table);
    if (file_index == -1) {
        throw std::runtime_error("File or directory not found");
    }

    // Update the name in the file table
    strncpy(file_table[file_index].name, new_path.c_str(), sizeof(file_table[file_index].name));
    file_table[file_index].name[sizeof(file_table[file_index].name) - 1] = '\0';

    // Write back the updated file table
    blkdevsim->write(sizeof(SuperBlock), sizeof(file_table), (const char*)file_table);

    std::cout << "Moved " << old_path << " to " << new_path << std::endl;
}



