#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include "window.hpp"
#include "../ui/imguilayer.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "directionallight.hpp"
#include "../renderer/swapchain.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/commandbuffer.hpp"
#include "../renderer/shadermanager.hpp"
#include "../renderer/texturemanager.hpp"
#include "../renderer/materialmanager.hpp"
#include "../renderer/gbuffer.hpp"
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

// forward declare and structs to pass app and camera together
// this fixes the window resize crash
class Application;
class Camera;

struct WindowUserData {
    Application* app;
    Camera* camera;
};

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    GatWindow window;
    WindowUserData windowUserData;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VmaAllocator allocator;

    std::unique_ptr<ShaderManager> shaderManager;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<SwapChain> swapChain;
    std::unique_ptr<TextureManager> textureManager;
    std::unique_ptr<MaterialManager> materialManager;
    std::unique_ptr<GBuffer> gbuffer;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<CommandBuffer> commandBuffer;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<DirectionalLight> directionalLight;
    std::unique_ptr<ImGuiLayer> imguiLayer;

    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    float lastFrameTime = 0.0f;

    void mainLoop();
    void initVulkan();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createShaderManager();
    void createCamera();
    void createSwapChain();
    void createTextureManager();
    void createMaterialManager();
    void createGBuffer();
    void createPipeline();
    void createCommandBuffer();
    void createScene();
    void createDirectionalLight();
    void drawFrame();
    void cleanup();
    void recreateSwapChain();
    void createImGuiLayer();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};