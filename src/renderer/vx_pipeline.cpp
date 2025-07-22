#include "vx_pipeline.hpp"
#include "vx_utils.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace VxEngine {

    void PipelineBuilder::clear() {
        _inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        _rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        _colorBlendAttachment = {};
        _multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        _layout = {};
        _depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        _renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        _colorFormat = VK_FORMAT_UNDEFINED;
        _stages.clear();
    }
    
    // Build an empty pipeline to fill with information.
    VkPipeline PipelineBuilder::build_pipeline(VkDevice device) {
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;
        
        // Currently only one viewport and scissor.
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        
        // Currently no color blending.
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.pNext = nullptr;

        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.pNext = nullptr;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &_colorBlendAttachment;

        // Clear VertexInputStateCreateInfo as we are not using it.
        VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &_renderInfo;

        pipelineInfo.stageCount = _stages.size();
        pipelineInfo.pStages = _stages.data();

        pipelineInfo.pVertexInputState = &_vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &_inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &_rasterizer;
        pipelineInfo.pMultisampleState = &_multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &_depthStencil;
        pipelineInfo.layout = _layout;

        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicState.pDynamicStates = &dynamicStates[0];
        dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);

        pipelineInfo.pDynamicState = &dynamicState;

        VkPipeline pipeline;
        if(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            std::cout << "Failed to create graphics pipeline." << std::endl;
            return VK_NULL_HANDLE;
        }

        return pipeline;
    }

    void PipelineBuilder::add_shader_stage(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint) {
        _stages.push_back(createShaderStageCreateInfo(stage, module, entryPoint));
    }

    void PipelineBuilder::set_input_topology(VkPrimitiveTopology topo) {
        _inputAssembly.topology = topo;
        _inputAssembly.primitiveRestartEnable = VK_FALSE; // Not using primitive restart currently.
    }

    void PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
        _rasterizer.polygonMode = mode;
        _rasterizer.lineWidth = 1.0f;
    }

    void PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
        _rasterizer.cullMode = cullMode;
        _rasterizer.frontFace = frontFace;
    }

    void PipelineBuilder::set_multisampling_none() {
        _multisampling.sampleShadingEnable = VK_FALSE;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        _multisampling.minSampleShading = 1.0f;
        _multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        _multisampling.alphaToCoverageEnable = VK_FALSE;
        _multisampling.alphaToOneEnable = VK_FALSE;
    }

    void PipelineBuilder::disable_blending() {
        _colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        _colorBlendAttachment.blendEnable = VK_FALSE;
    }

    void PipelineBuilder::set_color_attachment_format(VkFormat format) {
        _colorFormat = format;
        _renderInfo.colorAttachmentCount = 1;
        _renderInfo.pColorAttachmentFormats = &_colorFormat;
    }

    void PipelineBuilder::set_depth_format(VkFormat format) {
        _renderInfo.depthAttachmentFormat = format;
    }

    void PipelineBuilder::disable_depth_test() {
        _depthStencil.depthTestEnable = VK_FALSE;
        _depthStencil.depthWriteEnable = VK_FALSE;
        _depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        _depthStencil.depthBoundsTestEnable = VK_FALSE;
        _depthStencil.stencilTestEnable = VK_FALSE;
        _depthStencil.front = {};
        _depthStencil.back = {};
        _depthStencil.minDepthBounds = 0.0f;
        _depthStencil.maxDepthBounds = 1.0f;
    }

    // Load shader module, not part of pipeline builder.
    bool load_shader_module(const char* filePath, VkDevice device, VkShaderModule* module){
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        if(!file.is_open()){
            std::cerr << "Failed to open shader file: " << filePath << std::endl;
            return false;
        }
        
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        
        file.seekg(0);
        
        file.read((char*)(buffer.data()), fileSize);
        
        file.close();
        
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        
        createInfo.codeSize = buffer.size() * sizeof(uint32_t);
        createInfo.pCode = buffer.data();
        
        VkShaderModule shaderModule;
        VX_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module.");
        
        *module = shaderModule;
        return true;
    }

} // namespace VxEngine