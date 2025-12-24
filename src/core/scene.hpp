#pragma once

#include "../renderer/mesh.hpp"
#include "../renderer/materialmanager.hpp"
#include "../renderer/texturemanager.hpp"
#include "../renderer/commandbuffer.hpp"
#include "../renderer/indirectdrawing.hpp"
#include "../renderer/gpubuffer.hpp"
#include "../ui/primitives/aabb.hpp"
#include "vk_mem_alloc.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class Scene {
public:
    struct Model {
        std::unique_ptr<Mesh> mesh;
        std::string name;
        std::string folderPath;
        std::vector<AABB> submeshAABBs;
    };

    Scene(VmaAllocator allocator, CommandBuffer* commandBuffer,
          MaterialManager* materialManager, TextureManager* textureManager);
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // load a model from an asset folder
    void loadModel(const std::string& assetFolderPath, const std::string& modelName);

    // draw all models in the scene
    void drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                 MaterialManager* materialManager) const;

    // clear all models from the scene
    void clear();

    // remove a specific model by name
    bool removeModel(const std::string& modelName);

    // check if a model is loaded
    bool isModelLoaded(const std::string& modelName) const;

    size_t getModelCount() const { return models.size(); }
    const Model* getModel(size_t index) const;

    const std::unordered_map<const MaterialManager::Material*, MaterialBatch>& getMaterialBatches() const {
        return opaqueBatches;
    }

    const std::unordered_map<const MaterialManager::Material*, MaterialBatch>& getOpaqueBatches() const {
        return opaqueBatches;
    }

    const std::unordered_map<const MaterialManager::Material*, MaterialBatch>& getTransparentBatches() const {
        return transparentBatches;
    }

    bool hasTransparentObjects() const { return !transparentBatches.empty(); }

    // get transparent batches sorted back-to-front by depth
    std::vector<std::pair<const MaterialManager::Material*, const MaterialBatch*>>
        getSortedTransparentBatches(const glm::mat4& viewProj) const;

    // bind unified vertex/index buffers to command buffer
    void bindUnifiedBuffers(VkCommandBuffer commandBuffer) const;
    bool hasUnifiedBuffers() const { return unifiedVertexBuffer.getBuffer() != VK_NULL_HANDLE; }

private:
    VmaAllocator allocator;
    CommandBuffer* commandBuffer;
    MaterialManager* materialManager;
    TextureManager* textureManager;

    std::vector<Model> models;
    std::unordered_map<const MaterialManager::Material*, MaterialBatch> opaqueBatches;
    std::unordered_map<const MaterialManager::Material*, MaterialBatch> transparentBatches;

    GPUBuffer unifiedVertexBuffer;
    GPUBuffer unifiedIndexBuffer;

    void buildMaterialBatches();
    void buildUnifiedBuffers();
};
