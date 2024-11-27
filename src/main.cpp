#include "SDL2/SDL_video.h"
#include "SDL_video.h"
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

class HelloTriangleApplication {
    public:
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }
    private:
        SDL_Window* gWindow;

        void initWindow() {
            gWindow = SDL_CreateWindow("Vulkan Minecraft", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_VULKAN);
        }
        void initVulkan() {

        }
        void mainLoop() {

        }
        void cleanup() {

        }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
