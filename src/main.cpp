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
#include <fstream>

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

static bool checkValidationLayerSupport() {
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

static std::vector<char> readFile(const std::string& file_name) {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_name);
    }

    const int64_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    std::cout << "Size of " << file_name << ": " << file_size << std::endl;

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
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
        VkExtent2D mSwapchainExtent{};
        VkFormat mSwapFormat{};
        VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> mSwapchainImages;
        std::vector<VkImageView> mSwapchainViews;
        std::vector<VkFramebuffer> mSwapchainFrameBuffers;
        VkRenderPass mRenderPass = VK_NULL_HANDLE;
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
        VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
        VkCommandPool mCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
        VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore mRenderFinishSemaphore = VK_NULL_HANDLE;
        VkFence mFlightFence = VK_NULL_HANDLE;

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
            createSwapChainViews();
            createRenderPass();
            createGraphicsPipeline();
            createFrameBuffers();
            createCommandPool();
            createCommandBuffer();
            createSyncObjects();
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
            mSwapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(mLogicalDevice, mSwapChain, &imageCount, mSwapchainImages.data());

            mSwapchainExtent = extent;
            mSwapFormat = surfaceFormat.format;
        }

        void createSwapChainViews() {
            mSwapchainViews.resize(mSwapchainImages.size());
            for (int i = 0; i < mSwapchainImages.size(); i ++) {
                VkImageViewCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image = mSwapchainImages[i];
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format = mSwapFormat;

                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.baseMipLevel = 0;
                createInfo.subresourceRange.layerCount = 1;
                createInfo.subresourceRange.levelCount = 1;

                if (vkCreateImageView(mLogicalDevice, &createInfo, nullptr, &mSwapchainViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create swap chain image view");
                };
            }
        }

        void createRenderPass() {
            VkAttachmentDescription colorAttachment {};
            colorAttachment.format = mSwapFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorRef {};
            colorRef.attachment = 0;
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstSubpass = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo passCreate {};
            passCreate.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            passCreate.attachmentCount = 1;
            passCreate.pAttachments = &colorAttachment;
            passCreate.subpassCount = 1;
            passCreate.pSubpasses = &subpass;
            passCreate.dependencyCount = 1;
            passCreate.pDependencies = &dependency;

            if (vkCreateRenderPass(mLogicalDevice, &passCreate, nullptr, &mRenderPass) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create render pass!");
            }
        }

        void createGraphicsPipeline() {
            auto vert = readFile("Shaders/vert.spv");
            auto frag = readFile("Shaders/frag.spv");

            VkShaderModule vertShaderMod = createShaderModule(vert);
            VkShaderModule fragShaderMod = createShaderModule(frag);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderMod;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderMod;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            std::vector pipelineDynamics = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo dynamicCreateInfo {};
            dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicCreateInfo.dynamicStateCount = pipelineDynamics.size();
            dynamicCreateInfo.pDynamicStates = pipelineDynamics.data();

            VkPipelineVertexInputStateCreateInfo vertexInput {};
            vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInput.vertexAttributeDescriptionCount = 0;
            vertexInput.vertexBindingDescriptionCount = 0;

            VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(mSwapchainExtent.width);
            viewport.height = static_cast<float>(mSwapchainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.offset = {0,0};
            scissor.extent = mSwapchainExtent;

            VkPipelineViewportStateCreateInfo viewportState {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;

            VkPipelineLayoutCreateInfo pipelineLayoutCreate {};
            pipelineLayoutCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            if (vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutCreate, nullptr, &mPipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineCreate {};
            pipelineCreate.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreate.stageCount = 2;
            pipelineCreate.pStages = shaderStages;
            pipelineCreate.pVertexInputState = &vertexInput;
            pipelineCreate.pInputAssemblyState = &inputAssembly;
            pipelineCreate.pViewportState = &viewportState;
            pipelineCreate.pRasterizationState = &rasterizer;
            pipelineCreate.pMultisampleState = &multisampling;
            pipelineCreate.pColorBlendState = &colorBlending;
            pipelineCreate.pDynamicState = &dynamicCreateInfo;
            pipelineCreate.layout = mPipelineLayout;
            pipelineCreate.renderPass = mRenderPass;
            pipelineCreate.subpass = 0;

            if (vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreate, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(mLogicalDevice, vertShaderMod, nullptr);
            vkDestroyShaderModule(mLogicalDevice, fragShaderMod, nullptr);
        }

        VkShaderModule createShaderModule(const std::vector<char>& bytes) {
            VkShaderModuleCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = bytes.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(mLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create shader module");
            }
            return shaderModule;
        }

        void createFrameBuffers() {
            mSwapchainFrameBuffers.resize(mSwapchainViews.size());

            for (size_t i = 0; i < mSwapchainViews.size(); i++) {
                VkImageView attachments[] = {mSwapchainViews[i]};
                VkFramebufferCreateInfo framebuffer {};
                framebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer.renderPass = mRenderPass;
                framebuffer.attachmentCount = 1;
                framebuffer.pAttachments = attachments;
                framebuffer.width = mSwapchainExtent.width;
                framebuffer.height = mSwapchainExtent.height;
                framebuffer.layers = 1;

                if (vkCreateFramebuffer(mLogicalDevice, &framebuffer, nullptr, &mSwapchainFrameBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer");
                }
            }
        }

        void createCommandPool() {
            QueueFamilyIndicies queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

            VkCommandPoolCreateInfo poolInfo {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create command pool");
            }
        }

        void createCommandBuffer() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = mCommandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &mCommandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create command buffer");
            }
        }

        void createSyncObjects() {
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (
                vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
                vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mRenderFinishSemaphore) != VK_SUCCESS ||
                vkCreateFence(mLogicalDevice, &fenceInfo, nullptr, &mFlightFence) != VK_SUCCESS) {
                throw std::runtime_error("failed to create sync objects!");
            }
        }

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = mRenderPass;
            renderPassInfo.framebuffer = mSwapchainFrameBuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0,0};
            renderPassInfo.renderArea.extent = mSwapchainExtent;
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(mSwapchainExtent.width);
            viewport.height = static_cast<float>(mSwapchainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = mSwapchainExtent;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffer);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("Failed to end cmd buffer");
            }
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
                drawFrame();
            }
            vkDeviceWaitIdle(mLogicalDevice);
        }

        void drawFrame() {
            vkWaitForFences(mLogicalDevice, 1, &mFlightFence, VK_TRUE, UINT64_MAX);
            vkResetFences(mLogicalDevice, 1, &mFlightFence);

            uint32_t imageIndex;
            vkAcquireNextImageKHR(mLogicalDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

            vkResetCommandBuffer(mCommandBuffer, 0);
            recordCommandBuffer(mCommandBuffer, imageIndex);

            VkSubmitInfo submitInfo {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitfor[] = {mImageAvailableSemaphore};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitfor;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &mCommandBuffer;

            VkSemaphore signals[] = {mRenderFinishSemaphore};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signals;

            if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mFlightFence) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit draw cmd buffer");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signals;
            presentInfo.swapchainCount = 1;
            VkSwapchainKHR swapchains[] = {mSwapChain};
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;

            vkQueuePresentKHR(mPresentQueue, &presentInfo);

        }

        void cleanup() {
            vkDestroySemaphore(mLogicalDevice, mImageAvailableSemaphore, nullptr);
            vkDestroySemaphore(mLogicalDevice, mRenderFinishSemaphore, nullptr);
            vkDestroyFence(mLogicalDevice, mFlightFence, nullptr);
            vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
            for (auto framebuffer : mSwapchainFrameBuffers) {
                vkDestroyFramebuffer(mLogicalDevice, framebuffer, nullptr);
            }
            vkDestroyPipeline(mLogicalDevice, mGraphicsPipeline, nullptr);
            vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
            vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);
            for (VkImageView iView : mSwapchainViews) {
                vkDestroyImageView(mLogicalDevice, iView, nullptr);
            }
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
