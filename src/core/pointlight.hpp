#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "../renderer/gpubuffer.hpp"
#include "vk_mem_alloc.h"

struct PointLightUBO {
    alignas(16) glm::vec3 position;
    alignas(4) float intensity;
    alignas(16) glm::vec3 color;
    alignas(4) float padding;
};

class PointLight {
public:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    PointLight(VkDevice device, VmaAllocator allocator);
    ~PointLight();

    PointLight(const PointLight&) = delete;
    PointLight& operator=(const PointLight&) = delete;

    void cleanup(VmaAllocator allocator);
    void updateUniformBuffer(VmaAllocator allocator, uint32_t currentFrame);

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setColor(const glm::vec3& col) { color = col; }
    void setIntensity(float i) { intensity = i; }
    void setEnabled(bool e) { enabled = e; }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getColor() const { return color; }
    float getIntensity() const { return intensity; }
    bool isEnabled() const { return enabled; }

    float* getPositionPtr() { return &position.x; }
    float* getColorPtr() { return &color.x; }
    float* getIntensityPtr() { return &intensity; }

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorSet getDescriptorSet(uint32_t frame) const { return descriptorSets[frame]; }
    VkBuffer getBuffer(uint32_t frame) const { return uniformBuffers[frame].getBuffer(); }

private:
    VkDevice device;

    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    bool enabled;

    GPUBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    void createUniformBuffers(VmaAllocator allocator);
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
};
