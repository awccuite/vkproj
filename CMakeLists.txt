cmake_minimum_required(VERSION 3.20)
project(VulkanProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option for shader compilation
option(COMPILE_SHADERS "Compile GLSL shaders to SPIR-V" OFF)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

add_subdirectory(3rdparty)

# Minimal shader compilation
if(COMPILE_SHADERS)
    # Since glslc is in PATH, just use it directly
    set(GLSLC_EXECUTABLE glslc)
    
    message(STATUS "Using glslc from PATH")
    
    set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/shaders)
    set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    
    # Create output directory
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
    
    # Find all shader files
    file(GLOB SHADERS 
        ${SHADER_DIR}/*.vert 
        ${SHADER_DIR}/*.frag 
        ${SHADER_DIR}/*.comp)
    
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SPIRV ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
        
        # Use proper Windows path handling and quotes
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
            COMMAND ${GLSLC_EXECUTABLE} "${SHADER}" -o "${SPIRV}"
            COMMAND ${CMAKE_COMMAND} -E echo "Compiled: ${SHADER_NAME} -> ${SPIRV}"
            DEPENDS ${SHADER}
            VERBATIM)
        
        list(APPEND SPIRV_FILES ${SPIRV})
    endforeach()
    
    add_custom_target(shaders ALL DEPENDS ${SPIRV_FILES})
    
    # Copy shaders to where the executable expects them
    add_custom_command(TARGET shaders POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${CMAKE_CURRENT_BINARY_DIR}/src/renderer/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${SHADER_OUTPUT_DIR}
                ${CMAKE_CURRENT_BINARY_DIR}/src/renderer/shaders
        COMMENT "Copying shaders to src/renderer/shaders/")
endif()

add_library(renderer
    src/renderer/vx_image.cpp
    src/renderer/vx_renderer.cpp
    src/renderer/vx_deletionManager.cpp
    src/renderer/vx_descriptors.cpp
    src/renderer/vx_pipeline.cpp
    src/renderer/vx_buffers.cpp)
target_include_directories(renderer PUBLIC src/renderer)
target_link_libraries(renderer PUBLIC
    imgui
    vk-bootstrap::vk-bootstrap
    glm::glm
    VulkanMemoryAllocator
    Vulkan::Vulkan
    SDL3::SDL3)

if(COMPILE_SHADERS)
    add_dependencies(renderer shaders)
endif()

add_executable(VulkanProject src/main.cpp)
target_link_libraries(VulkanProject PRIVATE renderer)