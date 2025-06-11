#pragma once

#include "vk_include.hpp"

#include <vector>
#include <string>

class VulkanRenderer {
public:
    static VulkanRenderer& Get(); // Singleton renderer get
	
	// Engine control variables
	bool _isInitialized = false;
	int _frameNumber = 0;
	bool _windowMinizmized = false;

	// Window variables
	VkExtent2D _windowExtent{ 1700 , 900 };
    SDL_Window* _window;
	VkSurfaceKHR _surface;

	// Vulkan device variables
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkPhysicalDeviceProperties _deviceProperties; // Store device properties including version

	// Swapchain variables
	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	
	void init();
	void cleanup();
	void draw();
	void run();
	void print_vulkan_info(); // Function to print Vulkan version info

private:
	void init_window();
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();

	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	const int VK_VERSION_MAJOR_MIN = 1;
	const int VK_VERSION_MINOR_MIN = 3;
	const int VK_VERSION_PATCH_MIN = 0;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
};