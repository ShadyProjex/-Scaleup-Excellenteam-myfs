cmake_minimum_required(VERSION 3.10)

# Project Name
project(MyFileSystem)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(src)

# Source files
set(SOURCES
        src/blkdev.cpp
        src/myfs.cpp
        src/myfs_main.cpp
        src/vfs.cpp
)

# Executable
add_executable(myfs ${SOURCES})
