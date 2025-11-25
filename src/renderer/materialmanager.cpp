#include "materialmanager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

MaterialManager::MaterialManager(VkDevice device, TextureManager* textureManager)
    : device(device)
    , textureManager(textureManager)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)
{
    createDescriptorSetLayout();
}

MaterialManager::~MaterialManager() {
}

void MaterialManager::cleanup() {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }

    materials.clear();
    materialNameToIndex.clear();
}

void MaterialManager::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

VkDescriptorSetLayout MaterialManager::getDescriptorSetLayout() const {
    return descriptorSetLayout;
}

size_t MaterialManager::getMaterialCount() const {
    return materials.size();
}

const MaterialManager::Material* MaterialManager::getMaterial(uint32_t index) const {
    if (index >= materials.size()) {
        return nullptr;
    }
    return &materials[index];
}

const MaterialManager::Material* MaterialManager::getMaterialByName(const std::string& name) const {
    auto it = materialNameToIndex.find(name);
    if (it == materialNameToIndex.end()) {
        return nullptr;
    }
    return &materials[it->second];
}

void MaterialManager::loadMaterialsFromFile(const std::string& mtlFilePath, const std::string& textureBasePath) {
    materials.clear();
    materialNameToIndex.clear();

    parseMtlFile(mtlFilePath, textureBasePath);

    createDescriptorPool();
    createDescriptorSets();

    std::cout << "Loaded " << materials.size() << " materials from " << mtlFilePath << std::endl;
}

void MaterialManager::parseMtlFile(const std::string& mtlFilePath, const std::string& textureBasePath) {
    std::ifstream file(mtlFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open MTL file: " + mtlFilePath);
    }

    Material currentMaterial;
    bool hasMaterial = false;

    auto finalizeMaterial = [&]() {
        if (hasMaterial) {
            if (currentMaterial.diffuseTexture == nullptr) {
                currentMaterial.diffuseTexture = textureManager->getDefaultTexture();
            }

            materialNameToIndex[currentMaterial.name] = static_cast<uint32_t>(materials.size());
            materials.push_back(currentMaterial);
            hasMaterial = false;
        }
    };

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            finalizeMaterial();

            std::string materialName;
            iss >> materialName;

            currentMaterial = Material();
            currentMaterial.name = materialName;
            currentMaterial.diffuseTexture = nullptr;
            currentMaterial.descriptorSet = VK_NULL_HANDLE;
            hasMaterial = true;

            std::cout << "Found material: " << materialName << std::endl;
        }
        else if (token == "map_Kd" && hasMaterial) {
            std::string texturePath;
            iss >> texturePath;

            std::string fullPath = textureBasePath + texturePath;

            currentMaterial.diffuseTexture = textureManager->loadTexture(fullPath);
            std::cout << "  Loaded diffuse texture: " << fullPath << std::endl;
        }
        else if (token == "map_Disp" || token == "map_Bump" ||
                 token == "map_d" || token == "map_Ka") {
            continue;
        }
    }

    finalizeMaterial();

    file.close();

    if (materials.empty()) {
        std::cout << "Warning: No materials found in MTL file" << std::endl;
    }
}

void MaterialManager::createDescriptorPool() {
    if (materials.empty()) {
        std::cout << "Warning: No materials to create descriptor pool for" << std::endl;
        return;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(materials.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(materials.size());

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    std::cout << "Created descriptor pool for " << materials.size() << " materials" << std::endl;
}

void MaterialManager::createDescriptorSets() {
    if (materials.empty()) {
        std::cout << "Warning: No materials to create descriptor sets for" << std::endl;
        return;
    }

    std::vector<VkDescriptorSetLayout> layouts(materials.size(), descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(materials.size());
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSets(materials.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < materials.size(); ++i) {
        materials[i].descriptorSet = descriptorSets[i];

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = materials[i].diffuseTexture->imageView;
        imageInfo.sampler = textureManager->getSampler();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = materials[i].descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    std::cout << "Created and updated " << materials.size() << " descriptor sets" << std::endl;
}
