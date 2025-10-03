add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)

# IMGUI does not have its own cmakelists.txt, make our own here.
add_library(imgui STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp)
target_include_directories(imgui PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)
# Add the dependencies for imgui
target_link_libraries(imgui PUBLIC
	Vulkan::Vulkan
	SDL3::SDL3)

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include)
    
add_library(GPUOpen::VulkanMemoryAllocator ALIAS VulkanMemoryAllocator)