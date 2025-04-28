add_library(renderer STATIC
    vk_renderer.hpp
    vk_renderer.cpp
)

# Add UTF-8 support for MSVC 
if(MSVC)
    target_compile_options(renderer PRIVATE /utf-8)
endif()

target_include_directories(renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL3_INCLUDE_DIRS}
)

target_link_libraries(renderer PUBLIC
    Vulkan::Vulkan
    ${SDL3_LIBRARIES}
) 