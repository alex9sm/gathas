#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "panels/leftpanel.hpp"
#include "panels/bottompanel.hpp"
#include "panels/directionallightpanel.hpp"
#include "panels/rightpanel.hpp"
#include <memory>

class Scene;
class DirectionalLight;

class ImGuiLayer {
public:
    ImGuiLayer();
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice,
        VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue,
        VkRenderPass renderPass, uint32_t imageCount, Scene* scene, DirectionalLight* light);

    void cleanup();

    void beginFrame();
    void endFrame(float);
    void render(VkCommandBuffer commandBuffer);

    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;

private:
    VkDevice device;
    VkDescriptorPool imguiDescriptorPool;

    void createDescriptorPool(uint32_t imageCount);

    std::unique_ptr<LeftPanel> leftPanel;
    std::unique_ptr<BottomPanel> bottomPanel;
    std::unique_ptr<DirectionalLightPanel> directionalLightPanel;
    std::unique_ptr<RightPanel> rightPanel;
};