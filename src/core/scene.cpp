#include "scene.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>

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

        // compute AABB for each submesh
        const auto& vertices = model.mesh->getVertices();
        const auto& indices = model.mesh->getIndices();
        uint32_t submeshCount = model.mesh->getSubmeshCount();

        model.submeshAABBs.reserve(submeshCount);
        for (uint32_t i = 0; i < submeshCount; ++i) {
            const SubMesh& submesh = model.mesh->getSubmesh(i);
            AABB aabb = AABB::computeFromSubmesh(vertices, indices, submesh.indexOffset, submesh.indexCount);
            model.submeshAABBs.push_back(aabb);
        }

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
    for (const auto& [material, batch] : opaqueBatches) {
        if (material && material->descriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);

            // push material constants
            struct MaterialPushConstants {
                glm::vec4 diffuseColor;
                uint32_t hasTexture;
                uint32_t hasNormalMap;
                float dissolve;
                float padding;
            } pushConstants;

            pushConstants.diffuseColor = material->diffuseColor;
            pushConstants.hasTexture = material->hasTexture ? 1u : 0u;
            pushConstants.hasNormalMap = material->hasNormalMap ? 1u : 0u;
            pushConstants.dissolve = material->dissolve;
            pushConstants.padding = 0.0f;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
                0, sizeof(MaterialPushConstants), &pushConstants);
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
    for (auto& [material, batch] : opaqueBatches) {
        batch.cleanup(allocator);
    }
    opaqueBatches.clear();

    for (auto& [material, batch] : transparentBatches) {
        batch.cleanup(allocator);
    }
    transparentBatches.clear();

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
    for (auto& [material, batch] : opaqueBatches) {
        batch.cleanup(allocator);
    }
    opaqueBatches.clear();

    for (auto& [material, batch] : transparentBatches) {
        batch.cleanup(allocator);
    }
    transparentBatches.clear();

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

    uint32_t modelIndex = 0;
    for (const auto& model : models) {
        if (!model.mesh) {
            modelIndex++;
            continue;
        }

        const MeshOffsets& offsets = meshOffsets[model.mesh.get()];

        for (uint32_t i = 0; i < model.mesh->getSubmeshCount(); ++i) {
            const std::string& materialName = model.mesh->getMaterialName(i);
            const MaterialManager::Material* material = materialManager->getMaterialByName(materialName);

            bool isTransparent = material && material->hasAlpha;
            auto& targetBatches = isTransparent ? transparentBatches : opaqueBatches;

            auto it = targetBatches.find(material);
            if (it == targetBatches.end()) {
                MaterialBatch newBatch;
                newBatch.material = material;
                auto [insertIt, success] = targetBatches.emplace(material, std::move(newBatch));
                it = insertIt;
            }

            it->second.addDrawWithOffsets(model.mesh.get(), i, model.transform,
                offsets.globalVertexOffset, offsets.globalIndexOffset, modelIndex);
        }
        modelIndex++;
    }

    for (auto& [material, batch] : opaqueBatches) {
        batch.allocateBuffers(allocator);
    }

    for (auto& [material, batch] : transparentBatches) {
        batch.allocateBuffers(allocator);
    }

    std::cout << "Built " << opaqueBatches.size() << " opaque batches and "
              << transparentBatches.size() << " transparent batches with unified buffers ("
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

std::vector<std::pair<const MaterialManager::Material*, const MaterialBatch*>>
Scene::getSortedTransparentBatches(const glm::mat4& viewProj) const {
    std::vector<std::pair<const MaterialManager::Material*, const MaterialBatch*>> sortedBatches;

    for (const auto& [material, batch] : transparentBatches) {
        sortedBatches.emplace_back(material, &batch);
    }

    std::sort(sortedBatches.begin(), sortedBatches.end(),
        [this, &viewProj](const auto& a, const auto& b) {
            auto computeBatchDepth = [this, &viewProj](const MaterialBatch* batch) -> float {
                if (batch->drawCommands.empty()) return 0.0f;

                const auto& cmd = batch->drawCommands[0];
                if (!cmd.mesh) return 0.0f;

                // center of the submesh
                const auto& verts = cmd.mesh->getVertices();
                const auto& submesh = cmd.mesh->getSubmesh(cmd.submeshIndex);

                if (verts.empty()) return 0.0f;

                glm::vec3 center(0.0f);
                uint32_t sampleCount = 0;
                uint32_t step = std::max(1u, submesh.indexCount / 10);

                const auto& indices = cmd.mesh->getIndices();
                for (uint32_t i = submesh.indexOffset; i < submesh.indexOffset + submesh.indexCount; i += step) {
                    if (i < indices.size()) {
                        uint32_t idx = indices[i];
                        if (idx < verts.size()) {
                            center += verts[idx].pos;
                            sampleCount++;
                        }
                    }
                }

                if (sampleCount > 0) {
                    center /= static_cast<float>(sampleCount);
                }

                glm::vec4 clipPos = viewProj * glm::vec4(center, 1.0f);
                return clipPos.z / clipPos.w;
            };

            float depthA = computeBatchDepth(a.second);
            float depthB = computeBatchDepth(b.second);

            return depthA > depthB;
        });

    return sortedBatches;
}

/*
    Per-frame frustum culling: extracts frustum from viewProj, tests each submesh AABB
    against it, and updates material batch visible commands with only the visible draws.
*/
void Scene::updateCulling(const glm::mat4& viewProj, uint32_t frameIndex) {
    frustum.extractFromViewProj(viewProj);

    std::unordered_map<const MaterialManager::Material*, std::vector<VkDrawIndexedIndirectCommand>> opaqueVisible;
    std::unordered_map<const MaterialManager::Material*, std::vector<VkDrawIndexedIndirectCommand>> transparentVisible;

    uint32_t totalVisible = 0;
    uint32_t totalTested = 0;

    for (auto& [material, batch] : opaqueBatches) {
        auto& visibleCmds = opaqueVisible[material];
        visibleCmds.reserve(batch.drawCommands.size());

        for (const auto& cmd : batch.drawCommands) {
            totalTested++;

            if (cmd.modelIndex >= models.size()) continue;
            const Model& model = models[cmd.modelIndex];
            if (cmd.submeshIndex >= model.submeshAABBs.size()) continue;

            const AABB& localAABB = model.submeshAABBs[cmd.submeshIndex];
            bool visible = frustum.testAABB(localAABB, model.transform);

            if (visible) {
                visibleCmds.push_back(cmd.indirectCommand);
                totalVisible++;
            }
        }
    }

    for (auto& [material, batch] : transparentBatches) {
        auto& visibleCmds = transparentVisible[material];
        visibleCmds.reserve(batch.drawCommands.size());

        for (const auto& cmd : batch.drawCommands) {
            totalTested++;

            if (cmd.modelIndex >= models.size()) continue;
            const Model& model = models[cmd.modelIndex];
            if (cmd.submeshIndex >= model.submeshAABBs.size()) continue;

            const AABB& localAABB = model.submeshAABBs[cmd.submeshIndex];
            bool visible = frustum.testAABB(localAABB, model.transform);

            if (visible) {
                visibleCmds.push_back(cmd.indirectCommand);
                totalVisible++;
            }
        }
    }

    for (auto& [material, batch] : opaqueBatches) {
        batch.updateVisibleCommands(frameIndex, opaqueVisible[material]);
    }

    for (auto& [material, batch] : transparentBatches) {
        batch.updateVisibleCommands(frameIndex, transparentVisible[material]);
    }

    lastVisibleCount[frameIndex] = totalVisible;
}

void Scene::recordIndirectBufferCopies(VkCommandBuffer cmd, uint32_t frameIndex) {
    for (auto& [material, batch] : opaqueBatches) {
        batch.recordBufferCopy(cmd, frameIndex);
    }

    for (auto& [material, batch] : transparentBatches) {
        batch.recordBufferCopy(cmd, frameIndex);
    }
}
