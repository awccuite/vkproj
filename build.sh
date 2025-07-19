#!/bin/bash
# Make sure we're executing in an MSYS2 environment
echo "Starting build with MinGW GCC..."

# Check for clean flag
CLEAN_BUILD=false
if [[ "$1" == "-c" || "$1" == "-clean" ]]; then
    CLEAN_BUILD=true
    echo "Clean build requested..."
fi

# Handle clean build or incremental build
if [ "$CLEAN_BUILD" = true ]; then
    # Cleanup and rebuild everything including submodules
    echo "Clean build - removing existing build directory..."
    rm -rf build
    mkdir -p build
    cd build
    
    # Configure with the toolchain file (BUILD_SUBMODULES=ON by default)
    echo "Running CMake configuration with submodules..."
    cmake -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_TOOLCHAIN_FILE=../mingw-gcc-toolchain.cmake \
          -DCMAKE_VERBOSE_MAKEFILE=ON \
          -DBUILD_SUBMODULES=ON \
          ..
else
    # Incremental build - create build dir if it doesn't exist
    if [ ! -d "build" ]; then
        echo "Build directory doesn't exist, creating and configuring..."
        mkdir -p build
        cd build
        
        # Configure with the toolchain file (include submodules on first build)
        echo "Running CMake configuration with submodules (first build)..."
        cmake -DCMAKE_BUILD_TYPE=Debug \
              -DCMAKE_TOOLCHAIN_FILE=../mingw-gcc-toolchain.cmake \
              -DCMAKE_VERBOSE_MAKEFILE=ON \
              -DBUILD_SUBMODULES=ON \
              ..
    else
        echo "Incremental build - skipping submodules, building only your code..."
        cd build
        
        # Reconfigure to skip submodules for faster builds
        cmake -DCMAKE_BUILD_TYPE=Debug \
              -DCMAKE_TOOLCHAIN_FILE=../mingw-gcc-toolchain.cmake \
              -DCMAKE_VERBOSE_MAKEFILE=ON \
              -DBUILD_SUBMODULES=OFF \
              ..
    fi
fi

# Build with verbose output so we can see what's happening
echo "Building project..."
make VERBOSE=1