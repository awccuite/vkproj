add_library(renderer STATIC
    vk_renderer.hpp
    vk_renderer.cpp
    vx_utils.hpp
)

# Convert Windows paths to Unix paths if needed
string(REPLACE "C:" "/c" SDL3_INCLUDE_DIRS_UNIX "${SDL3_INCLUDE_DIRS}")

target_include_directories(renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL3_INCLUDE_DIRS_UNIX}
)

target_link_libraries(renderer PUBLIC
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
    vk-bootstrap::vk-bootstrap
) 