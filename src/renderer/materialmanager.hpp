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
        VkDescriptorSet descriptorSet;
        glm::vec4 diffuseColor;
        bool hasTexture;
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
