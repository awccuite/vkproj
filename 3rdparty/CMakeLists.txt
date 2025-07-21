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

# GLM configuration as header-only library
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "Build dynamic/static library" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build the test programs" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "Generate the install target" FORCE)
add_subdirectory(glm)

# Set BUILD_SHARED_LIBS for other dependencies
set(BUILD_SHARED_LIBS ON)

# Create interface library for easier linking
add_library(3rdparty INTERFACE)
target_link_libraries(3rdparty INTERFACE
    imgui
    vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
    glm::glm
)

# Set up include directories for clean header includes
target_include_directories(3rdparty INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap/src
    ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glm/glm
) 