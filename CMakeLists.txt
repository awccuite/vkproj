cmake_minimum_required(VERSION 3.20)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

# Use MSYS2 clang compiler
set(CMAKE_C_COMPILER "/c/msys64/mingw64/bin/clang.exe" CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/c/msys64/mingw64/bin/clang++.exe" CACHE STRING "C++ compiler" FORCE)

# Set prefix path for MSYS2 packages
list(APPEND CMAKE_PREFIX_PATH "/c/msys64/mingw64")

# Make sure compiler settings propagate to subprojects
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Set Vulkan SDK path
set(VULKAN_SDK_PATH "/c/msys64/home/Aidan/SDKS/Vulkan/1.4.309.0")
set(SDL_STATIC ON CACHE BOOL "Build a SDL static library" FORCE)

# Find pkgconfig
find_package(PkgConfig REQUIRED)

# Set Vulkan paths manually
set(Vulkan_INCLUDE_DIR "/c/msys64/home/Aidan/SDKS/Vulkan/1.4.309.0/Include")
set(Vulkan_LIBRARY "/c/msys64/home/Aidan/SDKS/Vulkan/1.4.309.0/Lib/vulkan-1.lib")

# Create Vulkan target manually
add_library(Vulkan::Vulkan SHARED IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_IMPLIB "${Vulkan_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}"
)

# Find SDL3 using pkg-config
pkg_check_modules(SDL3 REQUIRED sdl3)

# Add renderer directory
add_subdirectory(src/renderer)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

add_executable(SDLTest src/sdl_test.cpp)
target_link_libraries(SDLTest PRIVATE ${SDL3_LIBRARIES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${VULKAN_SDK_PATH}/Include"
    src/renderer
    ${SDL3_INCLUDE_DIRS}
    # "C:/msys64/mingw64/include/c++/14.2.0"
    # "C:/msys64/mingw64/include/c++/14.2.0/x86_64-w64-mingw32"
    # "C:/msys64/mingw64/include/c++/14.2.0/backward"
    # "C:/msys64/mingw64/include"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
    renderer
) 