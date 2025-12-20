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
        uint32_t mipLevels;
        bool hasAlpha;
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
    const Texture* loadTexture(const std::string& filepath, bool srgb = true);
    const Texture* getDefaultTexture() const;
    VkSampler getSampler() const { return textureSampler; }

private:
    void createTextureSampler();
    void createDefaultTexture();
    Texture createTextureFromFile(const std::string& filepath, bool srgb);
    void generateMipmaps(VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
    uint32_t calculateMipLevels(uint32_t width, uint32_t height);
};