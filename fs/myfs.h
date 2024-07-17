#ifndef __MYFS_H__
#define __MYFS_H__

#include <memory>
#include <vector>
#include <stdint.h>
#include "blkdev.h"

class MyFs {
public:
	MyFs(BlockDeviceSimulator *blkdevsim_);

	/**
	 * format method
	 * This function discards the current content in the blockdevice and
	 * create a fresh new MYFS instance in the blockdevice.
	 */
	void format();

	/**
	 * create_file method
	 * Creates a new file in the required path.
	 * @param path_str the file path (e.g. "/newfile")
	 * @param directory boolean indicating whether this is a file or directory
	 */
	void create_file(std::string path_str, bool directory);

	/**
	 * get_content method
	 * Returns the whole content of the file indicated by path_str param.
	 * Note: this method assumes path_str refers to a file and not a
	 * directory.
	 * @param path_str the file path (e.g. "/somefile")
	 * @return the content of the file
	 */
	std::string get_content(std::string path_str);

	/**
	 * set_content method
	 * Sets the whole content of the file indicated by path_str param.
	 * Note: this method assumes path_str refers to a file and not a
	 * directory.
	 * @param path_str the file path (e.g. "/somefile")
	 * @param content the file content string
	 */
	void set_content(std::string path_str, std::string content);

   /**
	 * list_dir method
	 * Returns a list of a files in a directory.
	 * Note: this method assumes path_str refers to a directory and not a
	 * file.
	 * @param path_str the file path (e.g. "/somedir")
	 * @return a vector (you need to change the return type in the function declaration)
	 */
	void list_dir(std::string path_str);

    /**
	 * remove_file method
	 * Removes a file from the filesystem.
	 * This function deletes the file entry from the file table and marks its blocks as free in the free block bitmap.
	 * @param path_str the file path (e.g. "/somefile")
	 */
    void remove_file(std::string path_str);

    /**
     * @brief Remove the directory at the specified path.
     *
     * @param dir_path The path of the directory to be removed.
     * @throw std::runtime_error if the directory removal fails.
     */
    void remove_directory(std::string dir_path);

    /**
     * @brief Move a file or directory from the old path to the new path.
     *
     * @param old_path The current path of the file or directory.
     * @param new_path The new path where the file or directory should be moved.
     * @throw std::runtime_error if the move operation fails.
     */
    void move_file(std::string old_path, std::string new_path);
    void create_root_directory();
private:

	/**
	 * This struct represents the first bytes of a myfs filesystem.
	 * It holds some magic characters and a number indicating the version.
	 * Upon class construction, the magic and the header are tested - if
	 * they both exist than the file is assumed to contain a valid myfs
	 * instance. Otherwise, the blockdevice is formated and a new instance is
	 * created.
	 */
	struct myfs_header {
		char magic[4];
		uint8_t version;
	};

    static const uint8_t CURR_VERSION = 0x03;
    static const char *MYFS_MAGIC;
    static const int BLOCK_SIZE = 4096;
    static const int MAX_FILES = 128;
    static const int MAX_BLOCKS = BlockDeviceSimulator::DEVICE_SIZE / BLOCK_SIZE;

    struct FileEntry {
        char name[32];
        uint32_t start_block;
        uint32_t size;
        bool is_directory;
        int parent_index; // Index of the parent directory in the file table
        int child_indices[MAX_FILES]; // Indices of children (files/directories)
        int num_children; // Number of children
    };

    struct SuperBlock {
        myfs_header header;
        uint32_t file_table_size;
        uint32_t free_block_bitmap_size;
    };

	BlockDeviceSimulator *blkdevsim;

    //helper functions
    /**
    * Finds the index of the first free entry in the file table.
    *
    * @param file_table The array of FileEntry structures representing the file table.
    * @return Index of the first free entry, or -1 if no free entry is found.
    */
    int find_free_file_entry(FileEntry file_table[]);

    /**
    * Finds the index of the file entry with the specified path in the file table.
    *
    * @param path_str The path of the file or directory to find.
    * @param file_table The array of FileEntry structures representing the file table.
    * @return Index of the file entry, or -1 if not found.
    */
    int find_file_entry(std::string path_str, FileEntry file_table[]);

    /**
    * Finds the index of the directory entry with the specified path in the file table.
    *
    * @param path_str The path of the directory to find.
    * @param file_table The array of FileEntry structures representing the file table.
    * @return Index of the directory entry, or -1 if not found or not a directory.
    */
    int find_directory_entry(std::string path_str, FileEntry file_table[]);

    /**
    * Reads the content of a file specified by its entry into a string.
    *
    * @param file_entry The FileEntry structure representing the file to read.
    * @param content Output parameter where the content of the file will be stored.
    */
    void read_file_content(const FileEntry& file_entry, std::string& content,const int blocksize);

    /**
    * Allocates contiguous blocks on the block device for storing a new file or directory.
    *
    * @param num_blocks The number of free contiguous blocks needed.
    * @return The starting block index of the allocated blocks.
    * @throws std::runtime_error if there is not enough free space to allocate blocks.
    */
    int allocate_blocks(int num_blocks);

    /**
    * Allocates blocks and writes content to the blocks for a file.
    *
    * @param file_entry The FileEntry structure representing the file to write.
    * @param content The content to write into the file.
    * @param blocksize The block size.
    * @return The starting block index of the allocated blocks.
    */
    int allocate_and_write_blocks(const FileEntry& file_entry, const std::string& content,const int blocksize);

    /**
    * Updates the size of a file entry after writing new content to the file.
    *
    * @param file_entry The FileEntry structure representing the file to update.
    * @param content_size The size of the new content written to the file.
    * @param start_block The starting block index of the allocated blocks.
    */
    void update_file_entry(FileEntry& file_entry, int content_size,const int start_block);

    /**
    * Clears a file entry by resetting its fields to default values.
    *
    * @param file_entry The FileEntry structure representing the file entry to clear.
    */
    void clear_file_entry(FileEntry& file_entry);

    /**
    * Clears the free block bitmap on the block device by marking all blocks as free.
    */
    void clear_free_block_bitmap(SuperBlock superblock);

    /**
    * Clears all data blocks on the block device by writing empty blocks to each block.
    */
    void clear_data_blocks();

    void clear_file_table();

    void free_allocated_blocks(const FileEntry& file_entry);


};

#endif // __MYFS_H__
