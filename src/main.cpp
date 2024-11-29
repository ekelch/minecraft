#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "vulkan/vulkan_core.h"
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <__config>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const bool enableValidationLayers = true;

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char * vLayer : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(vLayer, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

class HelloTriangleApplication {
    public:
        void run() {
            initSDL();
            openWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }
    private:
        SDL_Window* gWindow;
        VkInstance vkInstance;

        bool initSDL() {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
                return false;
            }
            if (!SDL_Vulkan_LoadLibrary(NULL)) {
                SDL_Log( "SDL could not load vulkan library! SDL Error: %s\n", SDL_GetError() );
                return false;
            }
            return true;
        }

        bool openWindow() {
            gWindow = SDL_CreateWindow("Vulkan Minecraft", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_VULKAN);
            if (gWindow == NULL) {
                printf("Failed to create window\n%s\n", SDL_GetError());
                return false;
            }
            return true;
        }
        void initVulkan() {
            createInstance();
        }
        void createInstance() {
            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("Validation layers enabled but not available");
            }
            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Minecraft Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;


            Uint32 count_instance_extensions;
            const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);
            if (instance_extensions == NULL) { printf("something bad"); }

            int extensionCount = count_instance_extensions + 1;
            const char **sdlExtensions = (const char **)SDL_malloc(extensionCount * sizeof(const char *));
            sdlExtensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            SDL_memcpy(&sdlExtensions[1], instance_extensions, count_instance_extensions * sizeof(const char*));

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = extensionCount;
            createInfo.ppEnabledExtensionNames = sdlExtensions;
            createInfo.enabledLayerCount = 0;

            if (vkCreateInstance(&createInfo, NULL, &vkInstance) != VK_SUCCESS) {
                std::runtime_error("failed to create vk instance");
            }
            if (vkInstance == NULL) {
                printf("it is null");
            }
            SDL_free(sdlExtensions);
        }
        void mainLoop() {
            SDL_Event e;
            bool quit = false;
            while (!quit) {
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_EVENT_QUIT) {
                        quit = true;
                    }
                }

            }
        }
        void cleanup() {
            vkDestroyInstance(vkInstance, nullptr);
            SDL_DestroyWindow(gWindow);
            gWindow = NULL;
            SDL_Quit();
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
