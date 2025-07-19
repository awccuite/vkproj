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
)

# Option to control submodule building (default ON for clean builds)
option(BUILD_SUBMODULES "Build 3rd party submodules" ON)

if(BUILD_SUBMODULES)
    message(STATUS "Building submodules...")
    add_subdirectory(3rdparty/vk-bootstrap)
    add_subdirectory(3rdparty/VulkanMemoryAllocator)
else()
    message(STATUS "Skipping submodules - using existing libraries")
    # Find the already-built libraries
    find_library(VK_BOOTSTRAP_LIB vk-bootstrap PATHS ${CMAKE_BINARY_DIR} REQUIRED)
    find_library(VMA_LIB VulkanMemoryAllocator PATHS ${CMAKE_BINARY_DIR} REQUIRED)
    
    # Create imported targets for the existing libraries
    add_library(vk-bootstrap::vk-bootstrap STATIC IMPORTED)
    set_target_properties(vk-bootstrap::vk-bootstrap PROPERTIES
        IMPORTED_LOCATION ${VK_BOOTSTRAP_LIB}
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vk-bootstrap/src"
    )
    
    add_library(GPUOpen::VulkanMemoryAllocator INTERFACE IMPORTED)
    set_target_properties(GPUOpen::VulkanMemoryAllocator PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/VulkanMemoryAllocator/include"
    )
endif()

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
    GPUOpen::VulkanMemoryAllocator
)