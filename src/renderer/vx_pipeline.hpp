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

    class PipelineBuilder {
        public:
            PipelineBuilder() { clear(); };

            std::vector<VkPipelineShaderStageCreateInfo> _stages;

            VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
            VkPipelineRasterizationStateCreateInfo _rasterizer;
            VkPipelineColorBlendAttachmentState _colorBlendAttachment;
            VkPipelineMultisampleStateCreateInfo _multisampling;
            VkPipelineLayout _layout;
            VkPipelineDepthStencilStateCreateInfo _depthStencil;
            VkPipelineRenderingCreateInfo _renderInfo;
            VkFormat _colorFormat;

            void clear();
            void clear_stages() { _stages.clear(); };

            VkPipeline build_pipeline(VkDevice device);

            void add_shader_stage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint);
            void set_input_topology(VkPrimitiveTopology topo);
            void set_polygon_mode(VkPolygonMode mode);
            void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
            void set_multisampling_none();
            void disable_blending();
            void set_color_attachment_format(VkFormat format);
            void set_depth_format(VkFormat format);
            void disable_depth_test();
    };

    // Load a shader module from a file.
    bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* module);
    
} // namespace VxEngine