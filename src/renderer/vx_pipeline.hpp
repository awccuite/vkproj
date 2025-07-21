#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

#include "../../3rdparty/glm/glm/glm.hpp"

namespace VxEngine {

    struct ComputePushConstants {
        glm::vec4 data1;
        glm::vec4 data2;
        glm::vec4 data3;
        glm::vec4 data4;
    };
    
    // Wrapper class for a compute pipeline.
    struct ComputePipeline {
        std::string name;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;

        ComputePushConstants data;
    };

    // Load a shader module from a file.
    bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* module);
    
} // namespace VxEngine