#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "../renderer/gpubuffer.hpp"
#include "vk_mem_alloc.h"

struct LightingUBO {
    alignas(16) glm::vec3 direction;
    alignas(4) float intensity;
    alignas(16) glm::vec3 color;
    alignas(4) float ambientIntensity;
    alignas(16) glm::vec3 ambientColor;
    alignas(4) float specularPower;
    alignas(16) glm::vec3 cameraPosition;
    alignas(4) float padding;
};

class DirectionalLight {
public:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    DirectionalLight(VkDevice device, VmaAllocator allocator);
    ~DirectionalLight();

    DirectionalLight(const DirectionalLight&) = delete;
    DirectionalLight& operator=(const DirectionalLight&) = delete;

    void cleanup(VmaAllocator allocator);
    void updateUniformBuffer(VmaAllocator allocator, uint32_t currentFrame, const glm::vec3& cameraPos);

    void setDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }
    void setColor(const glm::vec3& col) { color = col; }
    void setIntensity(float i) { intensity = i; }
    void setAmbientColor(const glm::vec3& col) { ambientColor = col; }
    void setAmbientIntensity(float i) { ambientIntensity = i; }
    void setSpecularPower(float p) { specularPower = p; }

    glm::vec3 getDirection() const { return direction; }
    glm::vec3 getColor() const { return color; }
    float getIntensity() const { return intensity; }
    glm::vec3 getAmbientColor() const { return ambientColor; }
    float getAmbientIntensity() const { return ambientIntensity; }
    float getSpecularPower() const { return specularPower; }

    float* getDirectionPtr() { return &direction.x; }
    float* getColorPtr() { return &color.x; }
    float* getIntensityPtr() { return &intensity; }
    float* getAmbientColorPtr() { return &ambientColor.x; }
    float* getAmbientIntensityPtr() { return &ambientIntensity; }
    float* getSpecularPowerPtr() { return &specularPower; }

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorSet getDescriptorSet(uint32_t frame) const { return descriptorSets[frame]; }
    VkBuffer getBuffer(uint32_t frame) const { return uniformBuffers[frame].getBuffer(); }

private:
    VkDevice device;

    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    glm::vec3 ambientColor;
    float ambientIntensity;
    float specularPower;

    GPUBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    void createUniformBuffers(VmaAllocator allocator);
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
};
