#include "scene.hpp"
#include <iostream>
#include <stdexcept>

Scene::Scene(VmaAllocator allocator, CommandBuffer* commandBuffer,
             MaterialManager* materialManager, TextureManager* textureManager)
    : allocator(allocator)
    , commandBuffer(commandBuffer)
    , materialManager(materialManager)
    , textureManager(textureManager)
{
}

Scene::~Scene() {
    clear();
}

void Scene::loadModel(const std::string& assetFolderPath, const std::string& modelName) {
    std::string objPath = assetFolderPath + "/" + modelName + ".obj";
    std::string mtlPath = assetFolderPath + "/" + modelName + ".mtl";
    std::string textureBasePath = assetFolderPath + "/";

    std::cout << "Loading model: " << modelName << " from " << assetFolderPath << std::endl;

    // load materials first
    try {
        materialManager->loadMaterialsFromFile(mtlPath, textureBasePath);
    }
    catch (const std::exception& e) {
        std::cout << "Warning: Failed to load materials: " << e.what() << std::endl;
    }

    // create and load mesh
    Model model;
    model.mesh = std::make_unique<Mesh>();
    model.name = modelName;
    model.folderPath = assetFolderPath;

    try {
        model.mesh->loadFromFile(objPath, allocator, commandBuffer);
        models.push_back(std::move(model));
        std::cout << "Successfully loaded model: " << modelName << " (Total models: " << models.size() << ")" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load mesh: " << e.what() << std::endl;
        throw;
    }
}

void Scene::drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                    MaterialManager* materialManager) const {
    for (const auto& model : models) {
        if (model.mesh) {
            model.mesh->bind(commandBuffer);

            // draw each submesh with its corresponding material
            for (uint32_t i = 0; i < model.mesh->getSubmeshCount(); ++i) {
                const std::string& materialName = model.mesh->getMaterialName(i);
                const MaterialManager::Material* material = materialManager->getMaterialByName(materialName);

                if (material && material->descriptorSet != VK_NULL_HANDLE) {
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);
                }

                model.mesh->draw(commandBuffer, i);
            }
        }
    }
}

void Scene::clear() {
    for (auto& model : models) {
        if (model.mesh) {
            model.mesh->destroy(allocator);
        }
    }
    models.clear();
    std::cout << "Scene cleared" << std::endl;
}

bool Scene::removeModel(const std::string& modelName) {
    for (auto it = models.begin(); it != models.end(); ++it) {
        if (it->name == modelName) {
            if (it->mesh) {
                it->mesh->destroy(allocator);
            }
            models.erase(it);
            std::cout << "Removed model: " << modelName << " (Remaining models: " << models.size() << ")" << std::endl;
            return true;
        }
    }
    std::cout << "Model not found: " << modelName << std::endl;
    return false;
}

bool Scene::isModelLoaded(const std::string& modelName) const {
    for (const auto& model : models) {
        if (model.name == modelName) {
            return true;
        }
    }
    return false;
}

const Scene::Model* Scene::getModel(size_t index) const {
    if (index >= models.size()) {
        return nullptr;
    }
    return &models[index];
}
