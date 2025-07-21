add_library(renderer STATIC
    vx_utils.hpp
    vx_image.hpp
    vx_image.cpp
    vx_renderer.hpp
    vx_renderer.cpp
    vx_deletionManager.hpp
    vx_deletionManager.cpp
    vx_descriptors.hpp
    vx_descriptors.cpp
    vx_pipeline.hpp
    vx_pipeline.cpp
)

# Convert Windows paths to Unix paths if needed
string(REPLACE "C:" "/c" SDL3_INCLUDE_DIRS_UNIX "${SDL3_INCLUDE_DIRS}")

target_include_directories(renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL3_INCLUDE_DIRS_UNIX}
    ${CMAKE_BINARY_DIR}/3rdparty
)

target_link_libraries(renderer PUBLIC
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
    3rdparty
)

# Include shader compilation
add_subdirectory(shaders)