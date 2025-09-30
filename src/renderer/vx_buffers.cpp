#include "vx_buffers.hpp"

namespace VxEngine {

AllocatedBuffer create_buffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocator& allocator) {
    VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocationSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaalocInfo = {};
    vmaalocInfo.usage = memoryUsage;
    vmaalocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    AllocatedBuffer newBuffer;
    VX_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaalocInfo, &newBuffer._buffer, &newBuffer._allocation, &newBuffer._allocationInfo), "Failure in create_buffer");

    return newBuffer;
}

void destroy_bufferr(const AllocatedBuffer& buffer, VmaAllocator& allocator){
    vmaDestroyBuffer(allocator, buffer._buffer, buffer._allocation);
}

} // namespace VxEngine