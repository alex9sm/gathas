#pragma once

#include <vulkan/vulkan.h>
#include "texturemanager.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

class MaterialManager {
public:
    struct Material {
        std::string name;
        const TextureManager::Texture* diffuseTexture;
        const TextureManager::Texture* normalTexture;
        VkDescriptorSet descriptorSet;
        glm::vec4 diffuseColor;
        float dissolve;         // MTL 'd' value (1.0 = opaque, 0.0 = fully transparent)
        float roughness;        // MTL 'Pr' value (0.0 = smooth/mirror, 1.0 = rough/matte)
        bool hasTexture;
        bool hasNormalMap;
        bool hasAlpha;          // true if texture has alpha or dissolve < 1.0
    };

    MaterialManager(VkDevice device, TextureManager* textureManager);
    ~MaterialManager();

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    void loadMaterialsFromFile(const std::string& mtlFilePath, const std::string& textureBasePath);
    const Material* getMaterial(uint32_t index) const;
    const Material* getMaterialByName(const std::string& name) const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;
    size_t getMaterialCount() const;
    void cleanup();

private:
    VkDevice device;
    TextureManager* textureManager;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<Material> materials;
    std::unordered_map<std::string, uint32_t> materialNameToIndex;

    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void parseMtlFile(const std::string& mtlFilePath, const std::string& textureBasePath);
};
