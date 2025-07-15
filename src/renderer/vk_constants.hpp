#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>

// At some point I want to make these includable via <>
// #include "../../3rd_party/vma/include/vk_mem_alloc.h"
// #include "../../3rd_party/fmt/include/fmt/core.h"

// #include "../../3rd_party/glm/glm/mat4x4.hpp"
// #include "../../3rd_party/glm/glm/vec4.hpp"

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
static void vx_default_error_handler(VkResult result) {
    // std::cout << "Detected Vulkan error: " << result << std::endl;
    throw std::runtime_error("\tVulkan error: " + std::string(string_VkResult(result)) + ", terminating program.");
}

// Detect and warn about errors to std::cout
static void vx_default_warn_handler(VkResult result) {
    std::cout << "Vulkan warning: " << string_VkResult(result) << " (continuing execution)" << std::endl;
}

// Ignore error
static void vx_ignore_handler(VkResult result) {
}

#define VX_CHECK(result) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_default_error_handler(_vx_result); \
        } \
    } while(0)

#define VX_WARN(result) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_default_warn_handler(_vx_result); \
        } \
    } while(0)

#define VX_CHECK_IGNORE(result) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            vx_ignore_handler(_vx_result); \
        } \
    } while(0)  

#define VX_CHECK_HANDLE(result, handler) \
    do { \
        VkResult _vx_result = (result); \
        if(_vx_result != VK_SUCCESS) { \
            (handler)(_vx_result); \
        } \
    } while(0)

}; // namespace VxEngine