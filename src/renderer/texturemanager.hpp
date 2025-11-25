#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "commandbuffer.hpp"
#include <string>
#include <unordered_map>

class TextureManager {
public:
    struct Texture {
        VkImage image;
        VmaAllocation allocation;
        VkImageView imageView;
        uint32_t width;
        uint32_t height;
    };

private:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VmaAllocator allocator;
    CommandBuffer* commandBuffer;

    std::unordered_map<std::string, Texture> textureCache;
    VkSampler textureSampler;
    Texture defaultTexture;

public:
    TextureManager(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator, CommandBuffer* commandBuffer);
    ~TextureManager();

    void cleanup();
    const Texture* loadTexture(const std::string& filepath);
    const Texture* getDefaultTexture() const;
    VkSampler getSampler() const { return textureSampler; }

private:
    void createTextureSampler();
    void createDefaultTexture();
    Texture createTextureFromFile(const std::string& filepath);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};