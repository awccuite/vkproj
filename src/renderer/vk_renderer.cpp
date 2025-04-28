#include "vk_renderer.hpp"
#include "vk_include.hpp"

#include <chrono>
#include <thread>
#include <cassert>
#include <iostream>

constexpr bool bUseValidationLayers = false;
VulkanRenderer* renderer = nullptr;

VulkanRenderer& VulkanRenderer::Get() {
    return *renderer;
}

void VulkanRenderer::init() {
    std::cout << "Initializing renderer" << std::endl;

    assert(renderer == nullptr);
    renderer = this;

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
    
    _isInitialized = true;
}

void VulkanRenderer::cleanup() {
    if(_isInitialized) {
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
    std::cout << "Drawing" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void VulkanRenderer::run() {
    std::cout << "Entering main loop" << std::endl;
    
    SDL_Event e;
    bool quit = false;

    // Just run a short event loop for testing
    for (int i = 0; i < 10 && !quit; i++) {
        // Poll for events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
                break;
            }
        }
        
        // Simple delay
        SDL_Delay(100);
        std::cout << "Frame " << i << std::endl;
    }
    
    std::cout << "Exiting main loop" << std::endl;
}
