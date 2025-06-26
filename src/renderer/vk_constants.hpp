#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

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

} // namespace VxEngine