#pragma once

#include <vector>
#include <string>
#include "vertex.hpp"
#include "gpubuffer.hpp"
#include "vk_mem_alloc.h"

class CommandBuffer;

struct SubMesh {
    uint32_t indexOffset;
    uint32_t indexCount;
    uint32_t materialIndex;

    SubMesh(uint32_t offset, uint32_t count, uint32_t matIndex = 0)
        : indexOffset(offset), indexCount(count), materialIndex(matIndex) {}
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void loadFromFile(const std::string& filepath, VmaAllocator allocator, CommandBuffer* commandBuffer);
    void destroy(VmaAllocator allocator);

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer, uint32_t submeshIndex) const;
    void drawAll(VkCommandBuffer commandBuffer) const;

    uint32_t getTotalIndexCount() const { return totalIndexCount; }
    uint32_t getSubmeshCount() const { return static_cast<uint32_t>(submeshes.size()); }

private:
    GPUBuffer vertexBuffer;
    GPUBuffer indexBuffer;
    std::vector<SubMesh> submeshes;
    uint32_t totalIndexCount;

    void processObjFile(const std::string& filepath,
        std::vector<Vertex>& outVertices,
        std::vector<uint32_t>& outIndices);
};