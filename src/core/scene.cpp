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

        buildMaterialBatches();
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load mesh: " << e.what() << std::endl;
        throw;
    }
}

void Scene::drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                    MaterialManager* materialManager) const {
    for (const auto& [material, batch] : materialBatches) {
        if (material && material->descriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);

            // push material constants
            struct MaterialPushConstants {
                glm::vec4 diffuseColor;
                uint32_t hasTexture;
            } pushConstants;

            pushConstants.diffuseColor = material->diffuseColor;
            pushConstants.hasTexture = material->hasTexture ? 1u : 0u;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
                0, 20, &pushConstants); // vec4 (16) + uint (4) = 20 bytes
        }

        for (const auto& cmd : batch.drawCommands) {
            if (cmd.mesh) {
                cmd.mesh->bind(commandBuffer);
                cmd.mesh->draw(commandBuffer, cmd.submeshIndex);
            }
        }
    }
}

void Scene::clear() {
    for (auto& [material, batch] : materialBatches) {
        batch.cleanup(allocator);
    }
    materialBatches.clear();

    unifiedVertexBuffer.destroy(allocator);
    unifiedIndexBuffer.destroy(allocator);

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

            buildMaterialBatches();
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

void Scene::buildMaterialBatches() {
    for (auto& [material, batch] : materialBatches) {
        batch.cleanup(allocator);
    }
    materialBatches.clear();
    unifiedVertexBuffer.destroy(allocator);
    unifiedIndexBuffer.destroy(allocator);

    if (models.empty()) {
        std::cout << "No models to batch" << std::endl;
        return;
    }

    struct MeshOffsets {
        uint32_t globalVertexOffset;
        uint32_t globalIndexOffset;
    };
    std::unordered_map<const Mesh*, MeshOffsets> meshOffsets;

    uint32_t totalVertices = 0;
    uint32_t totalIndices = 0;

    for (const auto& model : models) {
        if (!model.mesh) continue;

        meshOffsets[model.mesh.get()] = { totalVertices, totalIndices };
        totalVertices += model.mesh->getVertexCount();
        totalIndices += model.mesh->getTotalIndexCount();
    }

    if (totalVertices == 0 || totalIndices == 0) {
        std::cout << "No geometry to batch" << std::endl;
        return;
    }

    buildUnifiedBuffers();

    for (const auto& model : models) {
        if (!model.mesh) continue;

        const MeshOffsets& offsets = meshOffsets[model.mesh.get()];

        for (uint32_t i = 0; i < model.mesh->getSubmeshCount(); ++i) {
            const std::string& materialName = model.mesh->getMaterialName(i);
            const MaterialManager::Material* material = materialManager->getMaterialByName(materialName);

            auto it = materialBatches.find(material);
            if (it == materialBatches.end()) {
                MaterialBatch newBatch;
                newBatch.material = material;
                auto [insertIt, success] = materialBatches.emplace(material, std::move(newBatch));
                it = insertIt;
            }

            it->second.addDrawWithOffsets(model.mesh.get(), i, glm::mat4(1.0f),
                offsets.globalVertexOffset, offsets.globalIndexOffset);
        }
    }

    for (auto& [material, batch] : materialBatches) {
        batch.buildIndirectBuffer(allocator);
    }

    std::cout << "Built " << materialBatches.size() << " material batches with unified buffers ("
              << totalVertices << " vertices, " << totalIndices << " indices)" << std::endl;
}

void Scene::buildUnifiedBuffers() {
    std::vector<Vertex> allVertices;
    std::vector<uint32_t> allIndices;

    for (const auto& model : models) {
        if (!model.mesh) continue;

        const auto& meshVertices = model.mesh->getVertices();
        const auto& meshIndices = model.mesh->getIndices();

        allVertices.insert(allVertices.end(), meshVertices.begin(), meshVertices.end());
        allIndices.insert(allIndices.end(), meshIndices.begin(), meshIndices.end());
    }

    if (allVertices.empty() || allIndices.empty()) {
        return;
    }

    VkDeviceSize vertexBufferSize = sizeof(Vertex) * allVertices.size();
    unifiedVertexBuffer.create(allocator, vertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        allVertices.data(), commandBuffer);

    VkDeviceSize indexBufferSize = sizeof(uint32_t) * allIndices.size();
    unifiedIndexBuffer.create(allocator, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        allIndices.data(), commandBuffer);

    std::cout << "Created unified buffers: " << allVertices.size() << " vertices, "
              << allIndices.size() << " indices" << std::endl;
}

void Scene::bindUnifiedBuffers(VkCommandBuffer cmd) const {
    if (unifiedVertexBuffer.getBuffer() == VK_NULL_HANDLE) {
        return;
    }

    VkBuffer vertexBuffers[] = { unifiedVertexBuffer.getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, unifiedIndexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}
