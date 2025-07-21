#pragma once

#include "vx_utils.hpp"

#include <vector>
#include <span>

// File for creating abstractions around descriptor sets and layouts.

// This owns the allocator for descriptor sets,
// and is responsible for creating, defining, and allocating the sets.

// There is a singleton instance of this class owned by the renderer.
namespace VxEngine {

    // Builder for creating and allocating descriptor sets.
    class DescriptorManager {
        public:
            DescriptorManager() = default;
            ~DescriptorManager() = default;

            // Define PoolSizeRatio at the top level to avoid duplication
            struct PoolSizeRatio {
                VkDescriptorType type;
                float ratio;
            };

            struct DescriptorLayoutBuilder { // Multiple instances of this class.
                std::vector<VkDescriptorSetLayoutBinding> bindings;

                void addBinding(uint32_t binding, VkDescriptorType type);
                void clear();

                VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags);
            };

            struct DescriptorAllocator { // Single instance of allocator owned by manager.
                // Use the PoolSizeRatio from the parent class
                using PoolSizeRatio = DescriptorManager::PoolSizeRatio;

                VkDescriptorPool pool;

                void init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
                void clear_descriptors(VkDevice device);
                void destroy_pool(VkDevice device);

                VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
            };

            // Public wrapper methods for cleaner interface
            void init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
                allocator.init_pool(device, maxSets, poolRatios);
            }

            void clear_descriptors(VkDevice device) {
                allocator.clear_descriptors(device);
            }

            void destroy_pool(VkDevice device) {
                allocator.destroy_pool(device);
            }

            VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout) {
                return allocator.allocate(device, layout);
            }

            // Create a new layout builder for convenience
            DescriptorLayoutBuilder createLayoutBuilder() {
                return DescriptorLayoutBuilder{};
            }

            VkDescriptorSet drawImageDescritptors;
            VkDescriptorSetLayout drawImageDescriptorLayout;

            DescriptorAllocator allocator;
            VkDescriptorPool pool;
    };

} // namespace VxEngine