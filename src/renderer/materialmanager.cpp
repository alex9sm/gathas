#include "materialmanager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <array>

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
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

    // Binding 0: diffuse texture sampler
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    // Binding 1: normal map sampler
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

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
    size_t materialsBeforeLoad = materials.size();

    parseMtlFile(mtlFilePath, textureBasePath);

    size_t newMaterialsCount = materials.size() - materialsBeforeLoad;

    // recreate descriptor pool and sets to accommodate all materials
    if (newMaterialsCount > 0) {
        // destroy old descriptor pool if it exists
        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
            descriptorPool = VK_NULL_HANDLE;
        }

        // recreate pool and sets for all materials
        createDescriptorPool();
        createDescriptorSets();

        std::cout << "Loaded " << newMaterialsCount << " new materials from " << mtlFilePath
                  << " (Total: " << materials.size() << ")" << std::endl;
    }
    else {
        std::cout << "No new materials loaded from " << mtlFilePath << " (all already exist)" << std::endl;
    }
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
            // check if material already exists
            if (materialNameToIndex.find(currentMaterial.name) != materialNameToIndex.end()) {
                std::cout << "Material '" << currentMaterial.name << "' already exists, skipping" << std::endl;
                hasMaterial = false;
                return;
            }

            // if no texture was specified, use constant color mode
            if (currentMaterial.diffuseTexture == nullptr) {
                currentMaterial.hasTexture = false;
                currentMaterial.diffuseTexture = textureManager->getDefaultTexture(); // still need a texture for descriptor set
            } else {
                currentMaterial.hasTexture = true;
            }

            // if no normal map was specified, use default texture as placeholder
            if (currentMaterial.normalTexture == nullptr) {
                currentMaterial.hasNormalMap = false;
                currentMaterial.normalTexture = textureManager->getDefaultTexture();
            } else {
                currentMaterial.hasNormalMap = true;
            }

            // determine if material has alpha (texture alpha or dissolve < 1.0)
            currentMaterial.hasAlpha = (currentMaterial.dissolve < 1.0f) ||
                (currentMaterial.hasTexture && currentMaterial.diffuseTexture->hasAlpha);

            // clamp roughness to valid range
            if (currentMaterial.roughness < 0.0f) currentMaterial.roughness = 0.0f;
            if (currentMaterial.roughness > 1.0f) currentMaterial.roughness = 1.0f;

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
            currentMaterial.normalTexture = nullptr;
            currentMaterial.descriptorSet = VK_NULL_HANDLE;
            currentMaterial.diffuseColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // daefault magenta
            currentMaterial.dissolve = 1.0f; // fully opaque by default
            currentMaterial.roughness = 1.0f; // fully rough by default
            currentMaterial.hasTexture = false;
            currentMaterial.hasNormalMap = false;
            currentMaterial.hasAlpha = false;
            hasMaterial = true;

        }
        else if (token == "Kd" && hasMaterial) {
            float r, g, b;
            iss >> r >> g >> b;
            currentMaterial.diffuseColor = glm::vec4(r, g, b, 1.0f);
        }
        else if (token == "map_Kd" && hasMaterial) {
            std::string texturePath;
            iss >> texturePath;

            std::string fullPath = textureBasePath + texturePath;

            currentMaterial.diffuseTexture = textureManager->loadTexture(fullPath);
        }
        else if (token == "map_Bump" && hasMaterial) {
            std::string texturePath;
            iss >> texturePath;

            // -bm flag if present
            if (texturePath == "-bm") {
                float bumpMultiplier;
                iss >> bumpMultiplier >> texturePath;
            }

            std::string fullPath = textureBasePath + texturePath;
            currentMaterial.normalTexture = textureManager->loadTexture(fullPath, false);
        }
        else if (token == "d" && hasMaterial) {
            float d;
            iss >> d;
            // only use dissolve for values 0.01 to 0.99
            if (d > 0.01f && d < 1.0f) {
                currentMaterial.dissolve = d;
            }
        }
        else if (token == "Tr" && hasMaterial) {
            // transparency 0.0 = opaque, 1.0 = transparent
            float tr;
            iss >> tr;
            float d = 1.0f - tr;
            if (d > 0.01f && d < 1.0f) {
                currentMaterial.dissolve = d;
            }
        }
        else if ((token == "Pr" || token == "roughness") && hasMaterial) {
            float r;
            iss >> r;
            currentMaterial.roughness = r;
        }
        else if (token == "map_Disp" || token == "map_d" || token == "map_Ka") {
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

    // 2 samplers per material: diffuse + normal map
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(materials.size()) * 2;

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

        // diffuse texture (binding 0)
        VkDescriptorImageInfo diffuseImageInfo{};
        diffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        diffuseImageInfo.imageView = materials[i].diffuseTexture->imageView;
        diffuseImageInfo.sampler = textureManager->getSampler();

        // normal map (binding 1)
        VkDescriptorImageInfo normalImageInfo{};
        normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfo.imageView = materials[i].normalTexture->imageView;
        normalImageInfo.sampler = textureManager->getSampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = materials[i].descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &diffuseImageInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = materials[i].descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &normalImageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    std::cout << "Created and updated " << materials.size() << " descriptor sets" << std::endl;
}
