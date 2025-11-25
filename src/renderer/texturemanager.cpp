#include "texturemanager.hpp"
#include "commandbuffer.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureManager::TextureManager(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator, CommandBuffer* commandBuffer)
    : device(device)
    , physicalDevice(physicalDevice)
    , allocator(allocator)
    , commandBuffer(commandBuffer)
    , textureSampler(VK_NULL_HANDLE)
    , defaultTexture{}
{
    createTextureSampler();
    createDefaultTexture();
}

TextureManager::~TextureManager() {}

void TextureManager::cleanup() {
    for (auto& pair : textureCache) {
        Texture& texture = pair.second;
        if (texture.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, texture.imageView, nullptr);
        }
        if (texture.image != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator, texture.image, texture.allocation);
        }
    }
    textureCache.clear();

    if (defaultTexture.imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, defaultTexture.imageView, nullptr);
        defaultTexture.imageView = VK_NULL_HANDLE;
    }
    if (defaultTexture.image != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator, defaultTexture.image, defaultTexture.allocation);
        defaultTexture.image = VK_NULL_HANDLE;
        defaultTexture.allocation = VK_NULL_HANDLE;
    }
    if (textureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, textureSampler, nullptr);
        textureSampler = VK_NULL_HANDLE;
    }
}

void TextureManager::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}

void TextureManager::createDefaultTexture() {
    const uint32_t width = 1;
    const uint32_t height = 1;
    const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    const VkDeviceSize imageSize = width * height * 4;

    unsigned char pixels[4] = { 255, 0, 255, 255 };

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create staging buffer for default texture!");
    }

    void* data;
    vmaMapMemory(allocator, stagingAllocation, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingAllocation);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &defaultTexture.image, &defaultTexture.allocation, nullptr) != VK_SUCCESS) {
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
        throw std::runtime_error("Failed to create default texture image!");
    }

    defaultTexture.width = width;
    defaultTexture.height = height;

    VkCommandBuffer cmdBuffer = commandBuffer->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = defaultTexture.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer,
        defaultTexture.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer->endSingleTimeCommands(cmdBuffer);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = defaultTexture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &defaultTexture.imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create default texture image view!");
    }

    std::cout << "Created default texture (1x1 magenta)" << std::endl;
}

const TextureManager::Texture* TextureManager::getDefaultTexture() const {
    return &defaultTexture;
}

const TextureManager::Texture* TextureManager::loadTexture(const std::string& filepath) {
    auto it = textureCache.find(filepath);
    if (it != textureCache.end()) {
        std::cout << "Texture already cached: " << filepath << std::endl;
        return &it->second;
    }
    try {
        Texture texture = createTextureFromFile(filepath);
        textureCache[filepath] = texture;
        std::cout << "Loaded texture: " << filepath << " (" << texture.width << "x" << texture.height << ")" << std::endl;
        return &textureCache[filepath];
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: Failed to load texture '" << filepath << "': " << e.what() << std::endl;
        std::cerr << "Using default texture instead." << std::endl;
        return &defaultTexture;
    }
}

TextureManager::Texture TextureManager::createTextureFromFile(const std::string& filepath) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        throw std::runtime_error("failed to load texture: " + filepath);
    }

    VkDeviceSize imageSize = width * height * 4;
    const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
        stbi_image_free(pixels);
        throw std::runtime_error("Failed to create staging buffer for texture!");
    }
    void* data;
    vmaMapMemory(allocator, stagingAllocation, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingAllocation);

    Texture texture{};
    texture.width = static_cast<uint32_t>(width);
    texture.height = static_cast<uint32_t>(height);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texture.width;
    imageInfo.extent.height = texture.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &texture.image, &texture.allocation, nullptr) != VK_SUCCESS) {
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
        stbi_image_free(pixels);
        throw std::runtime_error("Failed to create texture image!");
    }

    VkCommandBuffer cmdBuffer = commandBuffer->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { texture.width, texture.height, 1 };

    vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer,
        texture.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer->endSingleTimeCommands(cmdBuffer);
    stbi_image_free(pixels);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &texture.imageView) != VK_SUCCESS) {
        vmaDestroyImage(allocator, texture.image, texture.allocation);
        throw std::runtime_error("Failed to create texture image view!");
    }

    return texture;
}

void TextureManager::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer cmdBuffer = commandBuffer->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition");
    }

    vkCmdPipelineBarrier(
        cmdBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer->endSingleTimeCommands(cmdBuffer);
}

void TextureManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer cmdBuffer = commandBuffer->beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        cmdBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    commandBuffer->endSingleTimeCommands(cmdBuffer);
}
