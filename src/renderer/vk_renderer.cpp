#include "vk_renderer.hpp"
#include "vk_constants.hpp"
#include "vx_utils.hpp"

#include <chrono>
#include <thread>
#include <cassert>
#include <iostream>
#include <cmath>

#include "3rdparty/vk-bootstrap/src/VkBootstrap.h"
#include "vulkan/vulkan_core.h"

namespace VxEngine {

constexpr bool useValidationLayers = true;
VulkanRenderer* renderer = nullptr;

VulkanRenderer& VulkanRenderer::Get() {
    return *renderer;
}

void VulkanRenderer::init() {
    std::cout << "Initializing renderer" << std::endl;

    assert(renderer == nullptr);
    renderer = this;

    init_window();
    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();
    
    print_vulkan_info();
    
    _isInitialized = true;
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
    VX_WARN(vkEnumerateInstanceVersion(&instanceVersion));
    std::cout << "System Vulkan Instance Version: " 
              << VK_VERSION_MAJOR(instanceVersion) << "."
              << VK_VERSION_MINOR(instanceVersion) << "."
              << VK_VERSION_PATCH(instanceVersion) << std::endl;

    vkb::InstanceBuilder builder;
    auto instance_result = builder.set_app_name("VkProject")
        .request_validation_layers(useValidationLayers)
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
}

void VulkanRenderer::create_swapchain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapchainBuilder(_physicalDevice, _device, _surface);

    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

        _swapchainExtent = vkbSwapchain.extent;
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

void VulkanRenderer::init_swapchain() {
    create_swapchain(_windowExtent.width, _windowExtent.height);
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
        VX_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

        // Create a command buffer for each frame.
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {}; // Initialize to zero.
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.pNext = nullptr;
        commandBufferAllocInfo.commandPool = _frames[i]._commandPool;
        commandBufferAllocInfo.commandBufferCount = 1;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // Allocate a command buffer for each frame in our
        // _frames[i]._commandBuffer array. This is where frameData is stored.
        // This is where we will record our commands.

        VX_CHECK(vkAllocateCommandBuffers(_device, &commandBufferAllocInfo, &_frames[i]._commandBuffer));
    }

    std::cout << "Initialized command structures" << std::endl;
}



// Init per frame synchronization structures.
void VulkanRenderer::init_sync_structures() {
    // Use constexpr to create compile time constants
    // that are used to initialize the default fence and semaphore structures.
    constexpr VkFenceCreateInfo fenceInfo = VxUtils::createFenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    constexpr VkSemaphoreCreateInfo semaphoreInfo = VxUtils::createSemaphoreInfo(0);

    for(int i = 0; i < LIVE_FRAMES; i++) {
        VX_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_frames[i]._inFlightFence));
        VX_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i]._swapchainSem));
        VX_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_frames[i]._renderSem));
    }
}

void VulkanRenderer::destroy_frame_data() {
    for(int i = 0; i < LIVE_FRAMES; i++) {
        vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

        vkDestroyFence(_device, _frames[i]._inFlightFence, nullptr);
        vkDestroySemaphore(_device, _frames[i]._swapchainSem, nullptr);
        vkDestroySemaphore(_device, _frames[i]._renderSem, nullptr);
    }
}

void VulkanRenderer::cleanup() {
    if(_isInitialized) {
        vkDeviceWaitIdle(_device);
        
        destroy_frame_data();
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

constexpr static VkCommandBufferBeginInfo beginCommandBufferInfo(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

void VulkanRenderer::draw() {
    // std::cout << "Drawing frame " << _frameNumber << std::endl;
    if(_windowMinizmized) { // Limit FPS when window is minimized.
        std::this_thread::sleep_for(std::chrono::milliseconds(UNFOCUSED_FPS_LIMIT_MS));
    }

    // Check the "current frame" (at start of loop, this would be the frame from the previous draw call)
    // Wait for the fence, then reset it.
    VX_CHECK(vkWaitForFences(_device, 1, &_frames[_frameNumber % LIVE_FRAMES]._inFlightFence, VK_TRUE, DEFAULT_TIMEOUT_NS));
    VX_CHECK(vkResetFences(_device, 1, &_frames[_frameNumber % LIVE_FRAMES]._inFlightFence));

    uint32_t swapchainImageIndex;
    VX_CHECK(vkAcquireNextImageKHR(_device, _swapchain, DEFAULT_TIMEOUT_NS, _frames[_frameNumber % LIVE_FRAMES]._swapchainSem, nullptr, &swapchainImageIndex));

    VkCommandBuffer commandBuffer = get_current_frame_data()._commandBuffer;
    VX_CHECK(vkResetCommandBuffer(commandBuffer, 0)); // Grab and reset the command buffer for the framedata at index.

    // Begin command buffer
    constexpr auto commandBufferBeginInfo = beginCommandBufferInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VX_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    // Transition the image layout to a general (unoptimized) layout.
    VxUtils::transitionImageLayout(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    constexpr VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    float b = static_cast<float>(std::abs(std::sin(static_cast<double>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) / 1000000000.0)));
    VkClearColorValue clearValue = {{0.0f, 0.0f, b, 1.0f}};

    VkImageSubresourceRange clearRange = VxUtils::createImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    // Clear the image with our clearValue (should sinusoudally change colors per frame)
    vkCmdClearColorImage(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // Make the image presentable (Draw with VK_IMAGE_LAYOUT_PRESENT_SRC_KHR), from general layout.
    VxUtils::transitionImageLayout(commandBuffer, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    VX_CHECK(vkEndCommandBuffer(commandBuffer));
    // End the command buffer.

    VkCommandBufferSubmitInfo commandBufferSubmitInfo = VxUtils::createCommandBufferSubmitInfo(commandBuffer);
    // Grab the previous frame's swapchain semaphore to wait on.
    VkSemaphoreSubmitInfo waitSemaphoreInfo = VxUtils::createSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, _frames[_frameNumber % LIVE_FRAMES]._swapchainSem);
    VkSemaphoreSubmitInfo signalSemaphoreInfo = VxUtils::createSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, _frames[_frameNumber % LIVE_FRAMES]._renderSem);
    
    VkSubmitInfo2 submitInfo = VxUtils::createSubmitInfo2(&commandBufferSubmitInfo, &signalSemaphoreInfo, &waitSemaphoreInfo);
    VX_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submitInfo, _frames[_frameNumber % LIVE_FRAMES]._inFlightFence));

    // Present the image to the screen.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;
    
    presentInfo.pWaitSemaphores = &get_current_frame_data()._renderSem;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VX_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    _frameNumber++;
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
                _windowMinizmized = true;
            }
            if(e.type == SDL_EVENT_WINDOW_RESTORED){
                std::cout << "Window restored" << std::endl;
                _windowMinizmized = false;
            }
        }
        
        // Simple delay
        draw();
    }
    
    std::cout << "Exiting main loop" << std::endl;
}

} // namespace VxEngine
