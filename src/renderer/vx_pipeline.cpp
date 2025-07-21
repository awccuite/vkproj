#include "vx_pipeline.hpp"
#include "vx_utils.hpp"

#include <fstream>
#include <iostream>
#include <vector>

bool VxEngine::load_shader_module(const char* filePath, VkDevice device, VkShaderModule* module){
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

