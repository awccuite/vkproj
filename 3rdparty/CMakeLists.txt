# Third-party dependencies

# ImGui
add_subdirectory(imgui)

# vk-bootstrap
add_subdirectory(vk-bootstrap)

# VulkanMemoryAllocator (header-only) - create interface target
add_library(GPUOpen::VulkanMemoryAllocator INTERFACE IMPORTED)
set_target_properties(GPUOpen::VulkanMemoryAllocator PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include"
)

# Create interface library for easier linking
add_library(3rdparty INTERFACE)
target_link_libraries(3rdparty INTERFACE
    imgui
    vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
)

# Set up include directories for clean header includes
target_include_directories(3rdparty INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap/src
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include
) 