#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_surface.h"
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
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 800;

const std::vector validationLayers = {
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
        SDL_Window* gWindow = nullptr;
        VkInstance gInstance = VK_NULL_HANDLE;
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VkDevice mLogicalDevice = VK_NULL_HANDLE;
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;
        VkQueue mPresentQueue = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;
        VkExtent2D mSwapExtent{};
        VkFormat mSwapFormat{};
        VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;

        const std::vector<const char*> requiredDeviceExtensions = {
            "VK_KHR_portability_subset",
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        struct QueueFamilyIndicies {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            [[nodiscard]] bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value();
            };
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        static bool initSDL() {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
                return false;
            }
            if (!SDL_Vulkan_LoadLibrary(nullptr)) {
                SDL_Log( "SDL could not load vulkan library! SDL Error: %s\n", SDL_GetError() );
                return false;
            }
            return true;
        }

        bool openWindow() {
            gWindow = SDL_CreateWindow("Vulkan Minecraft", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_VULKAN);
            if (gWindow == nullptr) {
                printf("Failed to create window\n%s\n", SDL_GetError());
                return false;
            }
            return true;
        }

        void initVulkan() {
            createInstance();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
        }

        void createSurface() {
            if (!SDL_Vulkan_CreateSurface(gWindow, gInstance, nullptr, &mSurface)) {
                std::cout << "Failed to create surface: " << std::endl << SDL_GetError() << std::endl;
                throw std::runtime_error("Failed to create surface\n");
            }
        }

        void createLogicalDevice() {
            QueueFamilyIndicies indices = findQueueFamilies(mPhysicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures logicalDeviceFeatures{};
            VkDeviceCreateInfo logicalDeviceCreateInfo{};
            logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            logicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
            logicalDeviceCreateInfo.pEnabledFeatures = &logicalDeviceFeatures;
            logicalDeviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();
            logicalDeviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

            if (enableValidationLayers) {
                logicalDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                logicalDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                logicalDeviceCreateInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(mPhysicalDevice, &logicalDeviceCreateInfo, nullptr, &mLogicalDevice) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!\n");
            }
            vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
            vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &mPresentQueue);
        }

        void setupDebugMessenger() const {
            printf("Enable Validation Layers?: %d\n", enableValidationLayers);
            if constexpr (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (vkCreateDebugUtilsMessengerEXT(gInstance, &createInfo, nullptr, nullptr) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        void pickPhysicalDevice() {
            uint32_t deviceCount = 0;

            vkEnumeratePhysicalDevices(gInstance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("Failed to find any physical GPUs with Vulkan support!\n");
            }

            std::vector<VkPhysicalDevice> physDevices(deviceCount);
            vkEnumeratePhysicalDevices(gInstance, &deviceCount, physDevices.data());

            for (const auto& device : physDevices) {
                if (isDeviceSuitable(device)) {
                    mPhysicalDevice = device;
                    break;
                }
            }

            if (mPhysicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find suitable physical device");
            }
        }

        bool isDeviceSuitable(VkPhysicalDevice physDevice) {
            QueueFamilyIndicies indicies = findQueueFamilies(physDevice);
            bool extSupported = checkDeviceExtensionsSupported(physDevice);
            bool swapChainAdequate = false;
            if (extSupported) {
                SwapChainSupportDetails details = querySwapChainSupport(physDevice);
                swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
            }
            return indicies.isComplete() && swapChainAdequate;
        }

        bool checkDeviceExtensionsSupported(VkPhysicalDevice physDevice) {
            uint32_t supportedExtCount;
            vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &supportedExtCount, nullptr);
            std::vector<VkExtensionProperties> supportedExtensions(supportedExtCount);
            vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &supportedExtCount, supportedExtensions.data());

            std::set<std::string> required(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

            for (const VkExtensionProperties supported: supportedExtensions) {
                required.erase(supported.extensionName);
            }
            return required.empty();
        }

        QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device) const {
            QueueFamilyIndicies indicies;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indicies.graphicsFamily = i;

                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
                    if (presentSupport) {
                        indicies.presentFamily = i;
                    }
                }
                if (indicies.isComplete()) {
                    break;
                }
                i++;
            }
            return indicies;
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
            }

            uint32_t presentModes;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModes, nullptr);
            if (presentModes != 0) {
                details.presentModes.resize(presentModes);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModes, details.presentModes.data());
            }

            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
            for (const auto& aFormat : availableFormats) {
                if (aFormat.format == VK_FORMAT_B8G8R8_SRGB && aFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return aFormat;
                }
            }
            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
            for (const auto& pMode : presentModes) {
                if (pMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return pMode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            int width, height;
            SDL_GetWindowSizeInPixels(gWindow, &width, &height);
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width), static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
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

            Uint32 sdlExtCount;
            const char * const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
            std::vector<const char*> extensions(sdlExtCount);
            for (int i = 0; i < sdlExtCount; i++) {
                extensions[i] = sdlExtensions[i];
            }
            extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = extensions.size();
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.enabledLayerCount = 0;
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

            printf("enabled extensions: %d\nvalidation enabled: %d\n", createInfo.enabledExtensionCount, enableValidationLayers);
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }

            if (vkCreateInstance(&createInfo, nullptr, &gInstance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create vk instance");
            }
        }

        void createSwapChain() {
            SwapChainSupportDetails swapChainDetails = querySwapChainSupport(mPhysicalDevice);
            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
            VkExtent2D extent = chooseSwapExtent(swapChainDetails.capabilities);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainDetails.presentModes);

            uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
            if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount) {
                imageCount = swapChainDetails.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = mSurface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndicies indices = findQueueFamilies(mPhysicalDevice);
            uint32_t queueIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices = nullptr;
            }

            createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create swapchain\n");
            }

            vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, swapChainImages.data());

            mSwapExtent = extent;
            mSwapFormat = surfaceFormat.format;
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
            vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
            vkDestroyDevice(mLogicalDevice, nullptr);
            vkDestroySurfaceKHR(gInstance, mSurface, nullptr);
            vkDestroyInstance(gInstance, nullptr);
            SDL_DestroyWindow(gWindow);
            gWindow = nullptr;
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
