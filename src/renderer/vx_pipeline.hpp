#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// TODO: Come back and refactor it to create a better pipeline builder

namespace VxEngine {

    bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* module);
    
} // namespace VxEngine