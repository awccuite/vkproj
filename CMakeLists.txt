cmake_minimum_required(VERSION 3.20)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set prefix path for MSYS2 packages
list(APPEND CMAKE_PREFIX_PATH "/c/msys64/mingw64")

# Set Vulkan SDK path
set(VULKAN_SDK_PATH "/c/msys64/home/Aidan/SDKS/Vulkan/1.4.309.0")
set(SDL_STATIC ON CACHE BOOL "Build a SDL static library" FORCE)

# Find pkgconfig
find_package(PkgConfig REQUIRED)

# Use pkg-config to find Vulkan
pkg_check_modules(VULKAN REQUIRED vulkan)

# Add 3rdparty dependencies
add_subdirectory(3rdparty)

# Convert Windows paths to Unix paths for MSYS2 environment
string(REPLACE "C:" "/c" VULKAN_INCLUDE_DIRS_UNIX "${VULKAN_INCLUDE_DIRS}")

# Create Vulkan target
add_library(Vulkan::Vulkan INTERFACE IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS_UNIX}"
    INTERFACE_LINK_LIBRARIES "${VULKAN_LIBRARIES}"
)

# Find SDL3 using pkg-config
pkg_check_modules(SDL3 REQUIRED sdl3)

# Add renderer directory
add_subdirectory(src/renderer)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${VULKAN_SDK_PATH}/Include"
    src/renderer
    ${SDL3_INCLUDE_DIRS}
    "/c/msys64/mingw64/include/c++/14.2.0"
    "/c/msys64/mingw64/include/c++/14.2.0/x86_64-w64-mingw32"
    "/c/msys64/mingw64/include/c++/14.2.0/backward"
    "/c/msys64/mingw64/include"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    3rdparty
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
    renderer
)

# Ensure SPIR-V binaries are built before we launch the executable.
add_dependencies(${PROJECT_NAME} compile_shaders) 