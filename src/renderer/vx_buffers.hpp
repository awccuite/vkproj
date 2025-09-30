#pragma once

#include "vx_utils.hpp"

namespace VxEngine {

struct AllocatedBuffer {
    VkBuffer _buffer; // Vulkan buffer handle
    VmaAllocation _allocation; // Metadata for the buffer and its allocation. Used to free the buffer.
    VmaAllocationInfo _allocationInfo;
};

AllocatedBuffer create_buffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocator& allocator);

void destroy_buffer(const AllocatedBuffer& buffer, VmaAllocator& allocator);

} // namespace VxEngine