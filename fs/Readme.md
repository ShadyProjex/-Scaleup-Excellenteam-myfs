# MyFileSystem Project

## Overview

This project is an implementation of a custom file system simulator. The file system is designed to run on a block device simulator and provides basic file system functionalities such as creating, reading, writing, and listing files and directories.

## Project Structure

The project has the following structure:
```
project/
├── build/
├── src/
│ ├── blkdev.cpp
│ ├── blkdev.h
│ ├── myfs.cpp
│ ├── myfs.h
│ ├── myfs_main.cpp
│ ├── vfs.cpp
│ ├── vfs.h
├── CMakeLists.txt
└── build_and_run.sh
```
### Source Files

- **blkdev.cpp/h**: Implementation of the block device simulator.
- **myfs.cpp/h**: Implementation of the custom file system.
- **myfs_main.cpp**: Entry point of the file system simulator application.
- **vfs.cpp/h**: Implementation of the virtual file system layer.

### Scripts

- **build_and_run.sh**: Script to build and run the project using CMake.

## Building and Running

### Prerequisites

- CMake 3.10 or higher
- A C++ compiler supporting C++11

### Steps to Build and Run

1. **Clone the repository:**

    ```sh
    git clone <repository_url>
    cd project
    ```

2. **Make the build script executable:**

    ```sh
    chmod +x build_and_run.sh
    ```

3. **Build and run the project:**

    ```sh
    ./build_and_run.sh
    ```

   This script will:
    - Create a `build` directory if it doesn't exist.
    - Change to the `build` directory.
    - Run `cmake ..` to configure the project.
    - Build the project using `make`.
    - Run the `myfs` executable.

## Usage

After running the `myfs` executable, you will be presented with a command-line interface where you can execute various file system commands:

- `touch <filename>`: Create a new file.
- `mkdir <dirname>`: Create a new directory.
- `ls <dirname>`: List the contents of a directory.
- `cat <filename>`: Display the contents of a file.
- `edit <filename>`: Edit the contents of a file.
- `rm <filename>`: Remove a file or directory.

### Example

```sh
myfs$ touch test
File test created.
myfs$ ls
test 0
myfs$ edit test
Enter new content (type EOF to finish):
Hello, World!
EOF
Content updated successfully.
myfs$ cat test
Hello, World!
```