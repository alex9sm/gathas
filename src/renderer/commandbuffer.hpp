#pragma once

#include <Vulkan/vulkan.h>
#include <vector>

class Mesh;

class CommandBuffer {
public:
    CommandBuffer(VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue);
    ~CommandBuffer();

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass,
        const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent, VkPipeline pipeline, Mesh* mesh);

    VkCommandBuffer getCommandBuffer(size_t index) const { return commandBuffers[index]; }
    const VkFence& getInFlightFence(size_t index) const { return inFlightFences[index]; }
    const VkSemaphore& getImageAvailableSemaphore(size_t index) const { return imageAvailableSemaphores[index]; }
    const VkSemaphore& getRenderFinishedSemaphore(size_t index) const { return renderFinishedSemaphores[index]; }

    size_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

    //transfer command buffer utilities
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