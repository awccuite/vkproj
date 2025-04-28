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

# Create Vulkan target
add_library(Vulkan::Vulkan INTERFACE IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${VULKAN_LIBRARIES}"
)

# Find SDL3 using pkg-config
pkg_check_modules(SDL3 REQUIRED sdl3)

# Add renderer directory
add_subdirectory(src/renderer)
add_subdirectory(vk-bootstrap)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

add_executable(SDLTest src/sdl_test.cpp)
target_link_libraries(SDLTest PRIVATE ${SDL3_LIBRARIES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${VULKAN_SDK_PATH}/Include"
    src/renderer
    ${SDL3_INCLUDE_DIRS}
    "C:/msys64/mingw64/include/c++/14.2.0"
    "C:/msys64/mingw64/include/c++/14.2.0/x86_64-w64-mingw32"
    "C:/msys64/mingw64/include/c++/14.2.0/backward"
    "C:/msys64/mingw64/include"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
    renderer
) 