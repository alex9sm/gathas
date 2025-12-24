#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "panels/leftpanel.hpp"
#include "panels/bottompanel.hpp"
#include "panels/directionallightpanel.hpp"
#include "panels/pointlightpanel.hpp"
#include "panels/camerapanel.hpp"
#include "panels/scenepanel.hpp"
#include "panels/rightpanel.hpp"
#include <memory>

class Scene;
class DirectionalLight;
class PointLight;
class Camera;

class ImGuiLayer {
public:
    ImGuiLayer();
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice,
        VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue,
        VkRenderPass renderPass, uint32_t imageCount, Scene* scene, DirectionalLight* light, PointLight* pointLight, Camera* camera);

    void cleanup();

    void beginFrame();
    void endFrame(float);
    void render(VkCommandBuffer commandBuffer);

    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;
    bool getShowAABBs() const { return leftPanel ? leftPanel->getShowAABBs() : false; }

private:
    VkDevice device;
    VkDescriptorPool imguiDescriptorPool;

    void createDescriptorPool(uint32_t imageCount);

    std::unique_ptr<LeftPanel> leftPanel;
    std::unique_ptr<BottomPanel> bottomPanel;
    std::unique_ptr<DirectionalLightPanel> directionalLightPanel;
    std::unique_ptr<PointLightPanel> pointLightPanel;
    std::unique_ptr<CameraPanel> cameraPanel;
    std::unique_ptr<ScenePanel> scenePanel;
    std::unique_ptr<RightPanel> rightPanel;
};