#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include "window.hpp"
#include "../renderer/swapchain.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/commandbuffer.hpp"
#include <Vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <optional>
#include <memory>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    GatWindow window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VmaAllocator allocator;

    std::unique_ptr<SwapChain> swapChain;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<CommandBuffer> commandBuffer;
    uint32_t currentFrame = 0;

    void mainLoop();
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createPipeline();
    void createCommandBuffer();
    void drawFrame();
    void cleanup();

    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};