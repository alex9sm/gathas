#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include "window.hpp"
#include "../renderer/swapchain.hpp"
#include <Vulkan/vulkan.h>
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

    std::unique_ptr<SwapChain> swapChain;

    void mainLoop();
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void cleanup();

    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};