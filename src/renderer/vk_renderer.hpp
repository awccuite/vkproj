#pragma once

#include "vk_include.hpp"

class VulkanRenderer {
public:
    static VulkanRenderer& Get(); // Singleton renderer get

	bool _isInitialized = false;
	int _frameNumber = 0;
	bool stop_rendering = false;
	VkExtent2D _windowExtent{ 1700 , 900 };
    SDL_Window* _window; // SDl window forward declaration

	//initializes everything in the engine
	void init();
	void cleanup();
	void draw();
	void run();
};