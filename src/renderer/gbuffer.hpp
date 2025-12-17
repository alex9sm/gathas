#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <vector>

class GBuffer {
public:
    static constexpr VkFormat ALBEDO_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
    static constexpr VkFormat NORMAL_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

private:
    VkDevice device;
    VmaAllocator allocator;
    VkExtent2D extent;

    VkImage albedoImage;
    VmaAllocation albedoAllocation;
    VkImageView albedoImageView;

    VkImage normalImage;
    VmaAllocation normalAllocation;
    VkImageView normalImageView;

    VkSampler sampler;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

public:
    GBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator, VkExtent2D extent);
    ~GBuffer();

    void cleanup();
    void recreate(VkExtent2D newExtent, VkImageView depthImageView);
    void updateDescriptorSets(VkImageView depthImageView);

    VkImageView getAlbedoImageView() const { return albedoImageView; }
    VkImageView getNormalImageView() const { return normalImageView; }
    VkSampler getSampler() const { return sampler; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorSet getDescriptorSet(uint32_t frameIndex) const { return descriptorSets[frameIndex]; }
    VkExtent2D getExtent() const { return extent; }

private:
    void createResources();
    void createImage(VkFormat format, VkImage& image, VmaAllocation& allocation);
    void createImageView(VkImage image, VkFormat format, VkImageView& imageView);
    void createSampler();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
};
