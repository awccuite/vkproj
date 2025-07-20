#pragma once

#include "vx_deletionManager.hpp"
#include "vx_utils.hpp"
#include "vx_image.hpp"
#include "vx_descriptors.hpp"
#include "vx_pipeline.hpp"

#include <cstdint>
#include <vector>

namespace VxEngine {

class VulkanRenderer {
public:
	static VulkanRenderer& Get(); // Singleton renderer get

	// Per frame data. Frames have their own command pool, command buffer, and synchronization structures.
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
	DeletionManager _engineDeletionManager; // Used to cleanup vulkan objects created for the renderer.

	// Draw image variables
	AllocatedImage _drawImage;
	VkExtent2D _drawExtent;

	// Pipelines for now
	VkPipeline _gradientPipeline;
	VkPipelineLayout _gradientPipelineLayout;
	
	void init();
	void cleanup();
    
	void run();
	
private:
	void init_window();
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void init_descriptors();
	void init_pipelines();
	void init_background_pipelines();

	void cleanup_vk_objects();

	void draw();
	void draw_background(VkCommandBuffer commandBuffer);

	void print_vulkan_info(); // Function to print Vulkan version info

	void create_swapchain();
	void create_draw_image();
	void destroy_swapchain();
	void destroy_frame_data();

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
	DescriptorManager _descriptorManager;
};

} // namespace VxEngine