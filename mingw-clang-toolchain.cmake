# This is a CMake toolchain file for MSYS2 MinGW GCC

# System name
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the compiler
set(CMAKE_C_COMPILER "/c/msys64/mingw64/bin/gcc.exe" CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/c/msys64/mingw64/bin/g++.exe" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_RC_COMPILER "/c/msys64/mingw64/bin/windres.exe" CACHE STRING "RC compiler" FORCE)

# Where is the target environment
set(CMAKE_FIND_ROOT_PATH "/c/msys64/mingw64")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and includes, only look in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Make sure pkg-config can find the .pc files
set(ENV{PKG_CONFIG_PATH} "/c/msys64/mingw64/lib/pkgconfig")

# Add extra flags if needed
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Avoid shared library issues
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++") 