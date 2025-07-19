#include "vx_descriptors.hpp"
#include "vulkan/vulkan_core.h"

namespace VxEngine {

    // DESCRIPTOR LAYOUT BUILDER ==================================================
    void DescriptorManager::DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type) {
        VkDescriptorSetLayoutBinding newBinding = {};
        newBinding.binding = binding;
        newBinding.descriptorType = type;
        newBinding.descriptorCount = 1;

        bindings.push_back(newBinding);
    }

    void DescriptorManager::DescriptorLayoutBuilder::clear() {
        bindings.clear();
    }

    VkDescriptorSetLayout DescriptorManager::DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags) {
        for(auto& binding : bindings) {
            binding.stageFlags |= shaderStages;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = pNext;

        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.flags = flags;

        VkDescriptorSetLayout setLayout;

        VX_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &setLayout), "Failed to create descriptor set layout.");

        return setLayout;
    }
    
    // DESCRIPTOR ALLOCATOR ======================================================
    void DescriptorManager::DescriptorAllocator::init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for(PoolSizeRatio ratio : poolRatios) {
            poolSizes.push_back(VkDescriptorPoolSize{ .type =ratio.type, .descriptorCount = static_cast<uint32_t>(ratio.ratio * maxSets) });
        }
        
        VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 
                                                .pNext = nullptr, 
                                                .flags = 0, 
                                                .maxSets = maxSets, 
                                                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()), .pPoolSizes = poolSizes.data() };

        VX_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool), "Failed to create descriptor pool.");
    }
    
    // Resets the descriptors created from the pool, but does not destroy the pool.
    void DescriptorManager::DescriptorAllocator::clear_descriptors(VkDevice device) {
        vkResetDescriptorPool(device, pool, 0);
    }

    // Destroys the pool.
    void DescriptorManager::DescriptorAllocator::destroy_pool(VkDevice device) {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    VkDescriptorSet DescriptorManager::DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 
                                                .pNext = nullptr, 
                                                .descriptorPool = pool, 
                                                .descriptorSetCount = 1,
                                                .pSetLayouts = &layout };

        VkDescriptorSet descriptorSet;
        VX_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet), "Failed to allocate descriptor set.");

        return descriptorSet;
    }

} // namespace VxEngine