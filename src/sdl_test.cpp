#include <SDL3/SDL.h>
#include <iostream>

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != true) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    std::cout << "SDL initialized successfully" << std::endl;
    
    // Create window
    SDL_Window* window = SDL_CreateWindow("SDL Test", 800, 600, SDL_WINDOW_VULKAN);
    
    if (window == NULL) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    std::cout << "Window created successfully" << std::endl;
    
    // Main loop
    SDL_Event e;
    bool quit = false;
    int frame = 0;
    
    while (!quit && frame < 100) {
        // Event handling
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        
        // Simple render
        frame++;
        std::cout << "Frame: " << frame << std::endl;
        SDL_Delay(50);
    }
    
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "SDL test completed successfully" << std::endl;
    return 0;
} 