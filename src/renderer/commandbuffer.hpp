#pragma once

#include <Vulkan/vulkan.h>
#include <vector>

class CommandBuffer {
public:
    CommandBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    ~CommandBuffer();

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
        VkRenderPass renderPass,
        const std::vector<VkFramebuffer>& framebuffers,
        VkExtent2D extent, VkPipeline pipeline);

    VkCommandBuffer getCommandBuffer(size_t index) const { return commandBuffers[index]; }
    const VkFence& getInFlightFence(size_t index) const { return inFlightFences[index]; }
    const VkSemaphore& getImageAvailableSemaphore(size_t index) const { return imageAvailableSemaphores[index]; }
    const VkSemaphore& getRenderFinishedSemaphore(size_t index) const { return renderFinishedSemaphores[index]; }

    size_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

private:
    VkDevice device;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores; 
    std::vector<VkFence> inFlightFences;             

    const int MAX_FRAMES_IN_FLIGHT = 2;

    void createCommandPool(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    void createCommandBuffers();
    void createSyncObjects();

    uint32_t findQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};