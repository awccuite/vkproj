#pragma once

#include "vx_utils.hpp"

#ifndef GLM_INCLUDE
#define GLM_INCLUDE
#include "../../3rdparty/glm/glm/glm.hpp"
#endif

namespace VxEngine {

// CPU Side representation of allocated buffer
struct AllocatedBuffer {
    VkBuffer _buffer; // Vulkan buffer handle
    VmaAllocation _allocation; // Metadata for the buffer and its allocation. Used to free the buffer.
    VmaAllocationInfo _allocationInfo;
};

// GPU Side Structs
struct Vertex {
    glm::vec3 pos;
    float uv_x;
    glm::vec3 norm;
    float uv_y;
    glm::vec4 col;
};

struct GPUMeshBuffer {
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertexBuffer;
    VkDeviceAddress vertexBufferAddress;
};

struct GPUDrawPushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBuffer;
};

AllocatedBuffer create_buffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocator& allocator);

void destroy_buffer(const AllocatedBuffer& buffer, VmaAllocator& allocator);

} // namespace VxEngine