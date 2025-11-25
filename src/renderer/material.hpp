#pragma once

#include "texturemanager.hpp"

struct Material {
    std::string name;
    const TextureManager::Texture* diffuseTexture;
    VkDescriptorSet descriptorSet;

    //future: add more properties
};

class MaterialManager {
private:
    VkDevice device;
    TextureManager* textureManager;
    VkDescriptorSetLayout materialDescriptorSetLayout;
    VkDescriptorPool materialDescriptorPool;

    std::vector<Material> materials;
    std::unordered_map<std::string, uint32_t> materialNameToIndex;

public:
    void loadMaterialsFromFile(const std::string& mtlFilePath,
        const std::string& textureBasePath);
    const Material* getMaterial(uint32_t index) const;

    VkDescriptorSetLayout getDescriptorSetLayout() const;
};