#include <vulkan/vulkan.h>
#include <iostream>

#include "renderer/vx_renderer.hpp"

int main(int argc, char* argv[]) {
    try {
        std::cout << "Starting main" << std::endl;

        // Get the renderer singleton and initialize it
        VxEngine::VulkanRenderer renderer;
        renderer.init();

        std::cout << "Running renderer" << std::endl;

        renderer.run();

        std::cout << "Cleaning up renderer" << std::endl;

        renderer.cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}