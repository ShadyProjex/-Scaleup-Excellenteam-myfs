#!/bin/bash

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Change to build directory
cd build

# Run CMake to configure the project
cmake ..

# Build the project
make

# Run the executable
./myfs
