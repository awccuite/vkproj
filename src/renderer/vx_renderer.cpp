#include "vx_renderer.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>

#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <optional>
#include <chrono>

// 3rd party includes that for some reason dont work with the cmake build system.
#define VMA_IMPLEMENTATION
#include "../../3rdparty/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include "../../3rdparty/vk-bootstrap/src/VkBootstrap.h"
#include "../../3rdparty/imgui/imgui.h"
#include "../../3rdparty/imgui/backends/imgui_impl_sdl3.h"
#include "../../3rdparty/imgui/backends/imgui_impl_vulkan.h"

namespace VxEngine {

constexpr bool USE_VALIDATION_LAYERS = true;
VulkanRenderer* renderer = nullptr;

VulkanRenderer& VulkanRenderer::Get() {
    return *renderer;
}

void VulkanRenderer::init() {
    std::cout << "Initializing renderer" << std::endl;

    assert(renderer == nullptr);
    renderer = this;

    init_window();
    std::cout << "Window initialized" << std::endl;
    init_vulkan();
    std::cout << "Vulkan initialized" << std::endl;
    init_swapchain();
    std::cout << "Swapchain initialized" << std::endl;
    init_commands();
    std::cout << "Commands initialized" << std::endl;
    init_sync_structures();
    std::cout << "Sync structures initialized" << std::endl;
    init_descriptors();
    std::cout << "Descriptors initialized" << std::endl;
    init_pipelines();
    std::cout << "Pipelines initialized" << std::endl;
    init_imgui();
    std::cout << "Imgui initialized" << std::endl;
    print_vulkan_info();

    _isInitialized = true;
    std::cout << "Renderer initialized" << std::endl;
}

// Function to print Vulkan version information
void VulkanRenderer::print_vulkan_info() {
    // Format Vulkan version from device properties
    uint32_t version = _deviceProperties.apiVersion;
    uint32_t major = VK_VERSION_MAJOR(version);
    uint32_t minor = VK_VERSION_MINOR(version);
    uint32_t patch = VK_VERSION_PATCH(version);
    
    std::cout << "------- Vulkan Info -------" << std::endl;
    std::cout << "Device: " << _deviceProperties.deviceName << std::endl;
    std::cout << "Vulkan Version: " << major << "." << minor << "." << patch << std::endl;
    std::cout << "Driver Version: " << _deviceProperties.driverVersion << std::endl;
    std::cout << "--------------------------" << std::endl;
}

void VulkanRenderer::init_window() {
    std::cout << "Initializing SDL" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != true) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        throw std::runtime_error("Failed to initialize SDL");
    }

    if (SDL_Vulkan_LoadLibrary(NULL) != true) {
        std::cerr << "SDL_Vulkan_LoadLibrary Error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("Failed to load Vulkan library");
    }

    std::cout << "Vulkan library loaded" << std::endl;

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    std::cout << "Creating window" << std::endl;

    _window = SDL_CreateWindow("Vulkan Test", _windowExtent.width, _windowExtent.height, window_flags);

    if (_window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
        throw std::runtime_error("Failed to create window");
    }

    std::cout << "Window created successfully" << std::endl;
}

void VulkanRenderer::init_vulkan() {
    // Query instance version before creating instance
    uint32_t instanceVersion = 0;
    VX_WARN(vkEnumerateInstanceVersion(&instanceVersion), "vkEnumerateInstanceVersion");
    std::cout << "System Vulkan Instance Version: " 
              << VK_VERSION_MAJOR(instanceVersion) << "."
              << VK_VERSION_MINOR(instanceVersion) << "."
              << VK_VERSION_PATCH(instanceVersion) << std::endl;

    vkb::InstanceBuilder builder;
    auto instance_result = builder.set_app_name("VkProject")
        .request_validation_layers(USE_VALIDATION_LAYERS)
        .require_api_version(VK_VERSION_MAJOR_MIN, VK_VERSION_MINOR_MIN, VK_VERSION_PATCH_MIN)
        .use_default_debug_messenger()
        .build();

    auto vkbInstance = instance_result.value();
    _instance = vkbInstance.instance;
    _debugMessenger = vkbInstance.debug_messenger;

    SDL_Vulkan_CreateSurface(_window, _instance, nullptr, &_surface);

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    
    vkb::PhysicalDeviceSelector selector(vkbInstance);
    vkb::PhysicalDevice physical_device = selector
        .set_minimum_version(VK_VERSION_MAJOR_MIN, VK_VERSION_MINOR_MIN)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(_surface)
        .select()
        .value();

    vkb::DeviceBuilder device_builder(physical_device);
    vkb::Device vkbDevice = device_builder.build().value();
    _device = vkbDevice.device;
    _physicalDevice = physical_device.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    
    // Get the physical device properties after selecting the device
    vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);

    // Initialize VMA
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VX_CHECK(vmaCreateAllocator(&allocatorInfo, &_allocator), "vmaCreateAllocator");

    _engineDeletionManager.push_function([this]() {
        vmaDestroyAllocator(_allocator);
    });
}

void VulkanRenderer::create_swapchain(/*uint32_t width, uint32_t height*/) {
    vkb::SwapchainBuilder swapchainBuilder(_physicalDevice, _device, _surface);

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(_windowExtent.width, _windowExtent.height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

        _swapchainExtent = vkbSwapchain.extent;
        _drawExtent = _swapchainExtent;
        _swapchain = vkbSwapchain.swapchain;
        _swapchainImages = vkbSwapchain.get_images().value();
        _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanRenderer::destroy_swapchain() {
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    for(int i = 0; i < _swapchainImageViews.size(); i++) {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

    _swapchainImageViews.clear();
    _swapchainImages.clear();
    _swapchain = nullptr;
}

void VulkanRenderer::create_draw_image() {
    VkExtent3D extent = {_drawExtent.width, _drawExtent.height, 1}; // 3D extent with 1 depth.

    _drawImage.format = VK_FORMAT_R16G16B16A16_SFLOAT; // High precision float format.
    _drawImage.extent = extent; // Assign our extent to the image. The drawImage has a 3d extent, while the swapchain has a 2d extent.

    VkImageUsageFlags drawImageUsageFlags{};
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo image_info = createImageCreateInfo(_drawImage.format, drawImageUsageFlags, extent);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image.
    VX_CHECK(vmaCreateImage(_allocator, &image_info, &allocInfo, &_drawImage.image, &_drawImage.allocation, nullptr), "vmaCreateImage");

    VkImageViewCreateInfo view_info = createImageViewCreateInfo(_drawImage.format, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
    VX_CHECK(vkCreateImageView(_device, &view_info, nullptr, &_drawImage.imageView), "vkCreateImageView");

    _engineDeletionManager.push_function([this]() {
        vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
        vkDestroyImageView(_device, _drawImage.imageView, nullptr);
    });
}

void VulkanRenderer::init_swapchain() {
    create_swapchain();
    std::cout << "Swapchain created" << std::endl;
    create_draw_image();
    std::cout << "Draw image created" << std::endl;
}

// TODO: Understand this better.
void VulkanRenderer::init_commands() {
    // Create a command pool for commands to be submitted to the graphics queue via.
    // Command pool features are enabled via flags. One feature we want is to reset command 
    // buffers without resetting the pool.

    // Use the same command pool info for all frames.
    VkCommandPoolCreateInfo commandPoolInfo = {}; // Initialize to zero.
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;

    for(int i = 0; i < LIVE_FRAMES; i++) {
        // Each frame has its own command pool, but they are configured identically.
        VX_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool), "failed to create frame command pool");

        // Create a command buffer for each frame.
        VkCommandBufferAllocateInfo commandBufferAllocInfo = createCommandBufferAllocateInfo(_frames[i]._commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Allocate a command buffer for each frame in our
        // _frames[i]._commandBuffer array. This is where frameData is stored.
        // This is where we will record our commands.

        VX_CHECK(vkAllocateCommandBuffers(_device, &commandBufferAllocInfo, &_frames[i]._commandBuffer), "failed to allocate frame command buffer");
    }

    VX_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_immCommandPool), "failed to create imgui command pool");

    VkCommandBufferAllocateInfo immCommandBufferAllocInfo = createCommandBufferAllocateInfo(_immCommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VX_CHECK(vkAllocateCommandBuffers(_device, &immCommandBufferAllocInfo, &_immCommandBuffer), "failed to allocate imgui command buffer");

    _engineDeletionManager.push_function([this]() {
        vkDestroyCommandPool(_device, _immCommandPool, nullptr);
        // vkFreeCommandBuffers(_device, _immCommandPool, 1, &_immCommandBuffer);
    });

    std::cout << "Initialized command structures" << std::endl;
}

// Init per frame synchronization structures.
void VulkanRenderer::init_sync_structures() {
    // Use constexpr to create compile time constants
    // that are used to initialize the default fence and semaphore structures.
    constexpr VkFenceCreateInfo fenceInfo = createFenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    constexpr VkSemaphoreCreateInfo semaphoreInfo = createSemaphoreInfo(0);

    for(int i = 0; i < LIVE_FRAMES; i++) {
        VX_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_frames[i]._inFlightFence), "vkCreateFence");
        VX_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i]._swapchainSem), "vkCreateSemaphore");
        VX_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i]._renderSem), "vkCreateSemaphore");
    }

    // Imgui fence.
    VX_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_immFence), "failed to create imgui fence");
    _engineDeletionManager.push_function([this]() {
        vkDestroyFence(_device, _immFence, nullptr);
    });
}

void VulkanRenderer::init_descriptors() {
    std::vector<DescriptorManager::PoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.0f }
    };
    // Allocate 10 descriptor sets for the draw image.
    _descriptorManager.init_pool(_device, 10, sizes);

    // Create a layout for the draw image descriptor set.
    auto layoutBuilder = _descriptorManager.createLayoutBuilder();
    layoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    _descriptorManager.drawImageDescriptorLayout = layoutBuilder.build(_device, VK_SHADER_STAGE_ALL, nullptr, 0);

    _descriptorManager.drawImageDescritptors = _descriptorManager.allocate(_device, _descriptorManager.drawImageDescriptorLayout);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = _drawImage.imageView;

    VkWriteDescriptorSet drawImageWriteDescriptorSet = {};
    drawImageWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWriteDescriptorSet.pNext = nullptr;

    drawImageWriteDescriptorSet.dstBinding = 0;
    drawImageWriteDescriptorSet.dstSet = _descriptorManager.drawImageDescritptors;
    // drawImageWriteDescriptorSet.dstArrayElement = 0;
    drawImageWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWriteDescriptorSet.descriptorCount = 1;
    drawImageWriteDescriptorSet.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(_device, 1, &drawImageWriteDescriptorSet, 0, nullptr);

    _engineDeletionManager.push_function([this]() {
        _descriptorManager.clear_descriptors(_device);
        _descriptorManager.destroy_pool(_device);
        vkDestroyDescriptorSetLayout(_device, _descriptorManager.drawImageDescriptorLayout, nullptr);
    });
}

void VulkanRenderer::init_pipelines() {
    init_background_pipelines();
}

void VulkanRenderer::init_background_pipelines() {
    VkPipelineLayoutCreateInfo computeLayoutInfo = {};
    computeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayoutInfo.pNext = nullptr;
    computeLayoutInfo.setLayoutCount = 1;
    computeLayoutInfo.pSetLayouts = &_descriptorManager.drawImageDescriptorLayout; // The descriptor layout currently owned by the descriptor manager.

    VX_CHECK(vkCreatePipelineLayout(_device, &computeLayoutInfo, nullptr, &_gradientPipelineLayout), "Compute pipeline layout creation failed.");

    VkShaderModule computeShaderModule;
    // Need to use relative to exe
    if(!load_shader_module("src/renderer/shaders/gradient.comp.spv", _device, &computeShaderModule)) {
        throw std::runtime_error("Failed to load compute shader module");
    }

    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.pNext = nullptr;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineInfo = {};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.pNext = nullptr;
    computePipelineInfo.stage = computeShaderStageInfo;
    computePipelineInfo.layout = _gradientPipelineLayout;

    VX_CHECK(vkCreateComputePipelines(_device, nullptr, 1, &computePipelineInfo, nullptr, &_gradientPipeline), "Compute pipeline creation failed.");

    vkDestroyShaderModule(_device, computeShaderModule, nullptr);

    _engineDeletionManager.push_function([this]() {
        vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);
        vkDestroyPipeline(_device, _gradientPipeline, nullptr);
    });
}

void VulkanRenderer::init_imgui() {
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
    
        VkDescriptorPool imguiPool;
        VX_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imguiPool), "failed to create imgui descriptor pool");
    
        // 2: initialize imgui library
    
        // this initializes the core structures of imgui
        ImGui::CreateContext();
    
        // this initializes imgui for SDL
        ImGui_ImplSDL3_InitForVulkan(_window);
    
        // this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = _instance;
        init_info.PhysicalDevice = _physicalDevice;
        init_info.Device = _device;
        init_info.Queue = _graphicsQueue;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.UseDynamicRendering = true;
    
        //dynamic rendering parameters for imgui to use
        init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_swapchainImageFormat;
        
    
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    
        ImGui_ImplVulkan_Init(&init_info);
        
        // add the destroy the imgui created structures
        _engineDeletionManager.push_function([=, this]() {
            ImGui_ImplVulkan_Shutdown();
            vkDestroyDescriptorPool(_device, imguiPool, nullptr);
        });
}

// Immediately submit command buffer without synchornization to the swapchain.
void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    VX_CHECK(vkResetFences(_device, 1, &_immFence), "failed to reset imgui fence");
    VX_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0), "failed to reset imgui command buffer");

    VkCommandBuffer imguiBuffer = _immCommandBuffer;
    VkCommandBufferBeginInfo imguiBeginInfo = beginCommandBufferInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VX_CHECK(vkBeginCommandBuffer(imguiBuffer, &imguiBeginInfo), "failed to begin imgui command buffer");

    function(imguiBuffer);

    VX_CHECK(vkEndCommandBuffer(imguiBuffer), "failed to end imgui command buffer");

    VkCommandBufferSubmitInfo imguiSubmitInfo = createCommandBufferSubmitInfo(imguiBuffer);
    VkSubmitInfo2 imguiSubmitInfo2 = createSubmitInfo2(&imguiSubmitInfo, nullptr, nullptr);

    VX_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &imguiSubmitInfo2, _immFence), "failed to submit imgui command buffer");
    VX_CHECK(vkWaitForFences(_device, 1, &_immFence, VK_TRUE, DEFAULT_TIMEOUT_NS), "failed to wait for imgui fence");
}

void VulkanRenderer::destroy_frame_data() {
    for(int i = 0; i < LIVE_FRAMES; i++) {
        vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
        vkDestroyFence(_device, _frames[i]._inFlightFence, nullptr);
        vkDestroySemaphore(_device, _frames[i]._swapchainSem, nullptr);
        vkDestroySemaphore(_device, _frames[i]._renderSem, nullptr);

        _frames[i].cleanup();
    }
}

void VulkanRenderer::cleanup_vk_objects() {
    _engineDeletionManager.delete_objects();
}

void VulkanRenderer::cleanup() {
    if(_isInitialized) {
        vkDeviceWaitIdle(_device);
        
        destroy_frame_data();
        cleanup_vk_objects();
        destroy_swapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);
        
        // Use vk-bootstrap's destroy_debug_utils_messenger function
        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
        vkDestroyInstance(_instance, nullptr);

        if (_window) {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }
        
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();
        _isInitialized = false;
    }

    renderer = nullptr;
}

void VulkanRenderer::draw() {
    // std::cout << "Drawing frame " << _frameNumber << std::endl;
    if(_windowMinimized) { // Limit FPS when window is minimized.
        std::this_thread::sleep_for(std::chrono::milliseconds(UNFOCUSED_FPS_LIMIT_MS));
    }

    // Check the "current frame" (at start of loop, this would be the frame from the previous draw call)
    // Wait for the fence, then reset it.
    VX_CHECK(vkWaitForFences(_device, 1, &get_current_frame_data()._inFlightFence, VK_TRUE, DEFAULT_TIMEOUT_NS), "vkWaitForFences");
    // After the frame is done, we can reset the fence and delete the frames objects.
    get_current_frame_data().cleanup();

    // Reset the fence for the current frame.
    VX_CHECK(vkResetFences(_device, 1, &get_current_frame_data()._inFlightFence), "vkResetFences");

    uint32_t swapchainImageIndex;
    VX_CHECK(vkAcquireNextImageKHR(_device, _swapchain, DEFAULT_TIMEOUT_NS, get_current_frame_data()._swapchainSem, nullptr, &swapchainImageIndex), "vkAcquireNextImageKHR");

    VkCommandBuffer commandBuffer = get_current_frame_data()._commandBuffer;
    VX_CHECK(vkResetCommandBuffer(commandBuffer, 0), "vkResetCommandBuffer"); // Grab and reset the command buffer for the framedata at index.

    // Begin first draw pass.
    constexpr auto commandBufferBeginInfo = beginCommandBufferInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VX_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "vkBeginCommandBuffer");

    // Transition the draw image to a general (unoptimized) layout.
    transitionImageLayout(commandBuffer, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    draw_background(commandBuffer); // Draw background to general.

    // Transition the draw image to a transfer source layout.
    transitionImageLayout(commandBuffer, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Transition the swapchain image to a transfer destination layout.
    transitionImageLayout(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy the draw image to the swapchain image.
    copyImageToImage(commandBuffer, _drawImage.image, _swapchainImages[swapchainImageIndex], _drawExtent, _swapchainExtent);

    // Transition the swapchain image to an attachment optimal layout for ImGui rendering.
    transitionImageLayout(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    
    // Draw ImGui debug overlay directly to swapchain (bypasses post-processing).
    draw_imgui(commandBuffer, _swapchainImageViews[swapchainImageIndex]);

    // Transition the swapchain image to presentable layout.
    transitionImageLayout(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VX_CHECK(vkEndCommandBuffer(commandBuffer), "vkEndCommandBuffer");
    // End imgui draw.
    // End the command buffer.

    VkCommandBufferSubmitInfo commandBufferSubmitInfo = createCommandBufferSubmitInfo(commandBuffer);
    // Grab the previous frame's swapchain semaphore to wait on.
    VkSemaphoreSubmitInfo waitSemaphoreInfo = createSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, get_current_frame_data()._swapchainSem);
    VkSemaphoreSubmitInfo signalSemaphoreInfo = createSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, get_current_frame_data()._renderSem);

    // Submit the command buffer to the graphics queue.
    VkSubmitInfo2 submitInfo = createSubmitInfo2(&commandBufferSubmitInfo, &signalSemaphoreInfo, &waitSemaphoreInfo);
    VX_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submitInfo, get_current_frame_data()._inFlightFence), "vkQueueSubmit2");

    // Present the image to the screen.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;
    
    presentInfo.pWaitSemaphores = &get_current_frame_data()._renderSem;
    presentInfo.waitSemaphoreCount = 1;

    // Present the image to the screen.
    presentInfo.pImageIndices = &swapchainImageIndex;
    VX_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo), "vkQueuePresentKHR");

    _frameNumber++;
}

// Draw the background image.
void VulkanRenderer::draw_background(VkCommandBuffer commandBuffer) { // Clear the draw image.
    constexpr VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    float b = static_cast<float>(std::sin(static_cast<double>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) / DEFAULT_TIMEOUT_NS) + 1.0f) / 2.0f;
    VkClearColorValue clearValue = {{0.0f, 0.0f, b , 1.0f}};

    VkImageSubresourceRange clearRange = createImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    // Clear the image with our clearValue (should sinusoudally change colors per frame)
    vkCmdClearColorImage(commandBuffer, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // Bind the compute gradient pipeline.
    // Bind the descriptor set containing the draw image.
    // Execute the compute pipeline.
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_descriptorManager.drawImageDescritptors, 0, nullptr);
    vkCmdDispatch(commandBuffer, std::ceil(_drawExtent.width / 16.0f), std::ceil(_drawExtent.height / 16.0f), 1);
}

void VulkanRenderer::draw_imgui(VkCommandBuffer commandBuffer, VkImageView imageView) {
    VkRenderingAttachmentInfo colorAttachment = createRenderingAttachmentInfo(imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = createRenderingInfo(_drawExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(commandBuffer, &renderInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRendering(commandBuffer);
}

void VulkanRenderer::run() {
    std::cout << "Entering main loop" << std::endl;

    SDL_Event e;
    bool quit = false;

    // Just run a short event loop for testing
    while(!quit) {
        // Poll for events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
                break;
            }
            
            if(e.type == SDL_EVENT_WINDOW_MINIMIZED){
                std::cout << "Window minimized" << std::endl;
                _windowMinimized = true;
            }
            if(e.type == SDL_EVENT_WINDOW_RESTORED){
                std::cout << "Window restored" << std::endl;
                _windowMinimized = false;
            }

            ImGui_ImplSDL3_ProcessEvent(&e); // Send event to imgui
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        
        draw();
    }
    
    std::cout << "Exiting main loop" << std::endl;
}

} // namespace VxEngine
