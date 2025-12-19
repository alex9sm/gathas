#include "directionallight.hpp"
#include <stdexcept>
#include <iostream>

DirectionalLight::DirectionalLight(VkDevice device, VmaAllocator allocator)
    : device(device)
    , direction(glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)))
    , color(1.0f, 1.0f, 1.0f)
    , intensity(1.0f)
    , ambientColor(1.0f, 1.0f, 1.0f)
    , ambientIntensity(0.1f)
    , specularPower(32.0f)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)
{
    createUniformBuffers(allocator);
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();

    std::cout << "directional light created" << std::endl;
}

DirectionalLight::~DirectionalLight() {
}

void DirectionalLight::cleanup(VmaAllocator allocator) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers[i].destroy(allocator);
    }

    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
}

void DirectionalLight::createUniformBuffers(VmaAllocator allocator) {
    VkDeviceSize bufferSize = sizeof(LightingUBO);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers[i].create(
            allocator,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            nullptr,
            nullptr
        );
    }
}

void DirectionalLight::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create light descriptor set layout");
    }
}

void DirectionalLight::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create light descriptor pool");
    }
}

void DirectionalLight::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate light descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(LightingUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void DirectionalLight::updateUniformBuffer(VmaAllocator allocator, uint32_t currentFrame, const glm::vec3& cameraPos) {
    LightingUBO ubo{};
    ubo.direction = direction;
    ubo.intensity = intensity;
    ubo.color = color;
    ubo.ambientIntensity = ambientIntensity;
    ubo.ambientColor = ambientColor;
    ubo.specularPower = specularPower;
    ubo.cameraPosition = cameraPos;
    ubo.padding = 0.0f;

    void* data;
    vmaMapMemory(allocator, uniformBuffers[currentFrame].getAllocation(), &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(allocator, uniformBuffers[currentFrame].getAllocation());
}
