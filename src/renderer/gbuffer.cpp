#include "gbuffer.hpp"
#include <stdexcept>

GBuffer::GBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VmaAllocator allocator, VkExtent2D extent)
    : device(device)
    , allocator(allocator)
    , extent(extent)
    , albedoImage(VK_NULL_HANDLE)
    , albedoAllocation(VK_NULL_HANDLE)
    , albedoImageView(VK_NULL_HANDLE)
    , normalImage(VK_NULL_HANDLE)
    , normalAllocation(VK_NULL_HANDLE)
    , normalImageView(VK_NULL_HANDLE)
    , roughnessImage(VK_NULL_HANDLE)
    , roughnessAllocation(VK_NULL_HANDLE)
    , roughnessImageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)
    , descriptorSets(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE)
{
    createResources();
    createDescriptorPool();
    createDescriptorSets();
}

GBuffer::~GBuffer() {}

void GBuffer::cleanup() {
    if (albedoImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, albedoImageView, nullptr);
        albedoImageView = VK_NULL_HANDLE;
    }
    if (albedoImage != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator, albedoImage, albedoAllocation);
        albedoImage = VK_NULL_HANDLE;
        albedoAllocation = VK_NULL_HANDLE;
    }

    if (normalImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, normalImageView, nullptr);
        normalImageView = VK_NULL_HANDLE;
    }
    if (normalImage != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator, normalImage, normalAllocation);
        normalImage = VK_NULL_HANDLE;
        normalAllocation = VK_NULL_HANDLE;
    }

    if (roughnessImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, roughnessImageView, nullptr);
        roughnessImageView = VK_NULL_HANDLE;
    }
    if (roughnessImage != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator, roughnessImage, roughnessAllocation);
        roughnessImage = VK_NULL_HANDLE;
        roughnessAllocation = VK_NULL_HANDLE;
    }

    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}

void GBuffer::recreate(VkExtent2D newExtent, VkImageView depthImageView) {
    cleanup();
    extent = newExtent;
    createResources();
    createDescriptorPool();
    createDescriptorSets();
    updateDescriptorSets(depthImageView);
}

void GBuffer::createResources() {
    createSampler();
    createDescriptorSetLayout();

    createImage(ALBEDO_FORMAT, albedoImage, albedoAllocation);
    createImageView(albedoImage, ALBEDO_FORMAT, albedoImageView);

    createImage(NORMAL_FORMAT, normalImage, normalAllocation);
    createImageView(normalImage, NORMAL_FORMAT, normalImageView);

    createImage(ROUGHNESS_FORMAT, roughnessImage, roughnessAllocation);
    createImageView(roughnessImage, ROUGHNESS_FORMAT, roughnessImageView);
}

void GBuffer::createImage(VkFormat format, VkImage& image, VmaAllocation& allocation) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GBuffer image!");
    }
}

void GBuffer::createImageView(VkImage image, VkFormat format, VkImageView& imageView) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GBuffer image view!");
    }
}

void GBuffer::createSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GBuffer sampler!");
    }
}

void GBuffer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 0;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.descriptorCount = 1;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalBinding{};
    normalBinding.binding = 1;
    normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.descriptorCount = 1;
    normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding depthBinding{};
    depthBinding.binding = 2;
    depthBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthBinding.descriptorCount = 1;
    depthBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    depthBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding roughnessBinding{};
    roughnessBinding.binding = 3;
    roughnessBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessBinding.descriptorCount = 1;
    roughnessBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    roughnessBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[] = { albedoBinding, normalBinding, depthBinding, roughnessBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GBuffer descriptor set layout!");
    }
}

void GBuffer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 4 * MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create GBuffer descriptor pool!");
    }
}

void GBuffer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate GBuffer descriptor sets!");
    }
}

void GBuffer::updateDescriptorSets(VkImageView depthImageView) {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo albedoInfo{};
        albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoInfo.imageView = albedoImageView;
        albedoInfo.sampler = sampler;

        VkDescriptorImageInfo normalInfo{};
        normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalInfo.imageView = normalImageView;
        normalInfo.sampler = sampler;

        VkDescriptorImageInfo depthInfo{};
        depthInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthInfo.imageView = depthImageView;
        depthInfo.sampler = sampler;

        VkDescriptorImageInfo roughnessInfo{};
        roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessInfo.imageView = roughnessImageView;
        roughnessInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrites[4]{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &albedoInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &normalInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &depthInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &roughnessInfo;

        vkUpdateDescriptorSets(device, 4, descriptorWrites, 0, nullptr);
    }
}
