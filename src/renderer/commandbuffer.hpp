#pragma once

#include <Vulkan/vulkan.h>
#include <vector>

class Scene;
class ImGuiLayer;
class MaterialManager;

class CommandBuffer {
public:
    CommandBuffer(VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue);
    ~CommandBuffer();

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    void recordGeometryPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent, VkPipeline pipeline, VkPipelineLayout pipelineLayout,
        VkDescriptorSet descriptorSet, MaterialManager* materialManager, Scene* scene);

    void recordLightingPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent, VkPipeline pipeline, VkPipelineLayout pipelineLayout,
        VkDescriptorSet cameraDescriptorSet, VkDescriptorSet gbufferDescriptorSet);

    void recordForwardPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent);

    void recordImGuiPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass, const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent, ImGuiLayer* imguiLayer);

    void recordFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkExtent2D extent,
        VkRenderPass geometryRenderPass, const std::vector<VkFramebuffer>& geometryFramebuffers,
        VkPipeline geometryPipeline, VkPipelineLayout geometryPipelineLayout,
        VkDescriptorSet cameraDescriptorSet, MaterialManager* materialManager, Scene* scene,
        VkRenderPass lightingRenderPass, const std::vector<VkFramebuffer>& lightingFramebuffers,
        VkPipeline lightingPipeline, VkPipelineLayout lightingPipelineLayout,
        VkDescriptorSet gbufferDescriptorSet,
        VkRenderPass forwardRenderPass, const std::vector<VkFramebuffer>& forwardFramebuffers,
        VkRenderPass imguiRenderPass, const std::vector<VkFramebuffer>& imguiFramebuffers,
        ImGuiLayer* imguiLayer);

    VkCommandBuffer getCommandBuffer(size_t index) const { return commandBuffers[index]; }
    const VkFence& getInFlightFence(size_t index) const { return inFlightFences[index]; }
    const VkSemaphore& getImageAvailableSemaphore(size_t index) const { return imageAvailableSemaphores[index]; }
    const VkSemaphore& getRenderFinishedSemaphore(size_t index) const { return renderFinishedSemaphores[index]; }

    size_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    VkDevice device;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    void createCommandPool(uint32_t graphicsQueueFamily);
    void createCommandBuffers();
    void createSyncObjects();
};