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
    const SubMesh& getSubmesh(uint32_t index) const { return submeshes[index]; }

    const std::string& getMaterialName(uint32_t submeshIndex) const;

    // access to vertex/index data for unified buffer building
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }
    uint32_t getVertexCount() const { return static_cast<uint32_t>(vertices.size()); }

private:
    GPUBuffer vertexBuffer;
    GPUBuffer indexBuffer;
    std::vector<SubMesh> submeshes;
    uint32_t totalIndexCount;

    // cpu data retained for unified buffer building
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void processObjFile(const std::string& filepath,
        std::vector<Vertex>& outVertices,
        std::vector<uint32_t>& outIndices);

    std::vector<std::string> materialNames;
};