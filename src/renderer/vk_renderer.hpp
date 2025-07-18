#pragma once

#include "vk_constants.hpp"
#include "vk_deletionManager.hpp"
#include "3rdparty/VulkanMemoryAllocator/include/vk_mem_alloc.h"

#include <cstdint>
#include <vector>

namespace VxEngine {

class VulkanRenderer {
public:
	static VulkanRenderer& Get(); // Singleton renderer get

	// Owned on a per frame basis, which lives in _frames
	// Maintains command data as well as synchronization structures.
    struct FrameData {
		// Command data
		VkCommandPool _commandPool;
		VkCommandBuffer _commandBuffer;

		// Synchronization structures

		// Semaphores are used for gpu -> gpu synchronzation
		// Fences are used for cpu -> gpu synchronization

		VkSemaphore _swapchainSem;
		VkSemaphore _renderSem;
		VkFence _inFlightFence;

		DeletionManager _deletionManager; // Used to cleanup per frame vulkan objects.

		void cleanup() { 
			_deletionManager.delete_objects();
		}
	};

	// Frame data and graphics queues
	FrameData _frames[LIVE_FRAMES];
	inline FrameData& get_current_frame_data() { return _frames[_frameNumber % LIVE_FRAMES]; };

    VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamilyIndex;
	
	// Engine control variables
	bool _isInitialized = false;
	bool _windowMinizmized = false;
	uint64_t _frameNumber = 0;

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

	VmaAllocator _allocator;
	DeletionManager _deletionManager; // Used to cleanup vulkan objects created for the renderer.
	
	void init();
	void cleanup();
    void cleanup_vk_objects();

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
	void destroy_frame_data();

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
};

} // namespace VxEngine