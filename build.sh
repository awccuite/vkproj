#!/bin/bash
# Make sure we're executing in an MSYS2 environment
echo "Starting build with MinGW GCC..."

# Cleanup and rebuild
rm -rf build
mkdir -p build
cd build

# Configure with the toolchain file
echo "Running CMake configuration..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=../mingw-gcc-toolchain.cmake \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      ..

# Build with verbose output so we can see what's happening
echo "Building project..."
make VERBOSE=1