#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include "window.hpp"
#include "../renderer/swapchain.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/commandbuffer.hpp"
#include "../renderer/shadermanager.hpp"
#include "../renderer/mesh.hpp"
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

    std::unique_ptr<ShaderManager> shaderManager;
    std::unique_ptr<SwapChain> swapChain;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<CommandBuffer> commandBuffer;
    std::unique_ptr<Mesh> mesh;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    void mainLoop();
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createShaderManager();
    void createSwapChain();
    void createPipeline();
    void createCommandBuffer();
    void createMesh();
    void drawFrame();
    void cleanup();
    void recreateSwapChain();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};