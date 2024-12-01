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
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

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

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        struct QueueFamilyIndicies {
            std::optional<uint32_t> graphicsFamily;
            bool isComplete() {
                return graphicsFamily.has_value();
            };
        };

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
            pickPhysicalDevice();
            createLogicalDevice();
        }

        void createLogicalDevice() {
            QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

            VkDeviceQueueCreateInfo logicalQueueCreateInfo {};
            logicalQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            logicalQueueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
            logicalQueueCreateInfo.queueCount = 1;
            float queuePriority = 1.0f;
            logicalQueueCreateInfo.pQueuePriorities = &queuePriority;

            VkPhysicalDeviceFeatures logicalDeviceFeatures;
            VkDeviceCreateInfo logicalDeviceCreateInfo;
            logicalDeviceCreateInfo.pQueueCreateInfos = &logicalQueueCreateInfo;
            logicalDeviceCreateInfo.queueCreateInfoCount = 1;
            logicalDeviceCreateInfo.pEnabledFeatures = &logicalDeviceFeatures;

            if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!\n");
            }
            vkGetDeviceQueue(logicalDevice, indicies.graphicsFamily.value(), 0, &graphicsQueue);
        }

        void setupDebugMessenger() {
            printf("Enable Validation Layers?: %d\n", enableValidationLayers);
            if (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (vkCreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, nullptr) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        void pickPhysicalDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("Failed to find any physical GPUs with Vulkan support!\n");
            }

            std::vector<VkPhysicalDevice> physDevices(deviceCount);
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physDevices.data());

            for (const auto& device : physDevices) {
                if (isDeviceSuitable(device)) {
                    physicalDevice = device;
                    break;
                }
            }

            if (physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find suitable physical device");
            }
        }

        bool isDeviceSuitable(VkPhysicalDevice physDevice) {
            QueueFamilyIndicies indicies = findQueueFamilies(physDevice);
            return indicies.isComplete();
        }

        QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device) {
            QueueFamilyIndicies indicies;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indicies.graphicsFamily = i;
                }
                if (indicies.isComplete()) {
                    break;
                }
                i++;
            }
            return indicies;
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

            printf("enabled extensions: %d\nvalidation enabled: %d\n", createInfo.enabledExtensionCount, enableValidationLayers);
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }

            if (vkCreateInstance(&createInfo, NULL, &vkInstance) != VK_SUCCESS) {
                std::runtime_error("failed to create vk instance");
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
            vkDestroyDevice(logicalDevice, nullptr);
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
