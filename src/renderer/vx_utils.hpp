#pragma once

#include <string>
#include <iostream>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// #include <fmt/core.h>
// #include <glm/mat4x4.hpp>
// #include <glm/vec4.hpp>

// Full include because it doesnt fucking work.
#include "../../3rdparty/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include "vx_image.hpp"

namespace VxEngine {

static constexpr uint32_t VK_VERSION_MAJOR_MIN = 1;
static constexpr uint32_t VK_VERSION_MINOR_MIN = 3;
static constexpr uint32_t VK_VERSION_PATCH_MIN = 0;

static constexpr uint32_t LIVE_FRAMES = 2; // Maximum number of frames to be preparing at any moment.

static constexpr uint32_t UNFOCUSED_FPS_LIMIT = 10;
static constexpr uint32_t UNFOCUSED_FPS_LIMIT_MS = 1000/UNFOCUSED_FPS_LIMIT;

static constexpr uint64_t INFINITE_TIMEOUT = UINT64_MAX;
static constexpr uint64_t DEFAULT_TIMEOUT_NS = 1000000000; // 1 second
static constexpr uint64_t DEFAULT_TIMEOUT_MS = DEFAULT_TIMEOUT_NS / 1000000;

// Detect and throw error
static void vx_default_error_handler(VkResult result, const char* functionName) {
    std::cout << "Vulkan error in function: " << functionName << " - " << string_VkResult(result) << std::endl;
    throw std::runtime_error("\tVulkan error: " + std::string(string_VkResult(result)) + " in function " + std::string(functionName) + ", terminating program.");
}

// Detect and warn about errors to std::cout
static void vx_default_warn_handler(VkResult result, const char* functionName) {
    std::cout << "Vulkan warning in function: " << functionName << " - " << string_VkResult(result) << " (continuing execution)" << std::endl;
}

// Ignore error
static void vx_ignore_handler(VkResult result, const char* functionName) {}

#define VX_CHECK(result, functionName) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_default_error_handler(_vx_result, functionName); \
        } \
    } while(0)

#define VX_WARN(result, functionName) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_default_warn_handler(_vx_result, functionName); \
        } \
    } while(0)

#define VX_CHECK_IGNORE(result, functionName) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_ignore_handler(_vx_result, functionName); \
        } \
    } while(0)  

#define VX_CHECK_HANDLE(result, handler) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            (handler)(_vx_result); \
        } \
    } while(0)

constexpr static VkFenceCreateInfo createFenceInfo(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = flags;

    return fenceCreateInfo;
}

constexpr static VkSemaphoreCreateInfo createSemaphoreInfo(VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = flags;

    return semaphoreCreateInfo;
}

constexpr static VkImageSubresourceRange createImageSubresourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subImage {};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return subImage;
}

constexpr static VkSemaphoreSubmitInfo createSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore){
    VkSemaphoreSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.semaphore = semaphore;
    info.stageMask = stageMask;
    info.deviceIndex = 0;
    info.value = 1;

    return info;
}

constexpr static VkCommandBufferSubmitInfo createCommandBufferSubmitInfo(VkCommandBuffer commandBuffer){
    VkCommandBufferSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = nullptr;
    info.commandBuffer = commandBuffer;
    info.deviceMask = 0;

    return info;
}

constexpr static VkCommandBufferAllocateInfo createCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBufferLevel level){
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = commandPool;
    info.commandBufferCount = commandBufferCount;
    info.level = level;

    return info;
}

// Takes in a command buffer info, a signal semaphore info, and a wait semaphore info.
constexpr static VkSubmitInfo2 createSubmitInfo2(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo){
    VkSubmitInfo2 info = {};

    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;
    info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = waitSemaphoreInfo;

    info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signalSemaphoreInfo;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = cmd;

    return info;
}

constexpr static VkCommandBufferBeginInfo beginCommandBufferInfo(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

constexpr static VkImageCreateInfo createImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlages, VkExtent3D extent){
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlages;

    return info;
}

constexpr static VkImageViewCreateInfo createImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags){
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

constexpr static VkRenderingAttachmentInfo createRenderingAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout){
    VkRenderingAttachmentInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    info.pNext = nullptr;

    info.imageView = view;
    info.imageLayout = layout;
    info.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if(clear){
        info.clearValue = *clear;
    }

    return info;
}

constexpr static VkRenderingInfo createRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment){
    VkRenderingInfo renderInfo {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, renderExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    return renderInfo;
}

constexpr static VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint) {
    VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;

		//shader stage
		info.stage = stage;
		//module containing the code for this shader stage
		info.module = module;
		//the entry point of the shader
		info.pName = "main";
		return info;
}

constexpr static VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    // empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

} // namespace VxEngine 