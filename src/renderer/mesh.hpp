#pragma once

#include <vector>
#include "vertex.hpp"
#include "gpubuffer.hpp"
#include "vk_mem_alloc.h"

class CommandBuffer;

class Mesh {
public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void createCube(VmaAllocator allocator, CommandBuffer* commandBuffer);
    void destroy(VmaAllocator allocator);

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer) const;

    uint32_t getIndexCount() const { return indexCount; }

private:
    GPUBuffer vertexBuffer;
    GPUBuffer indexBuffer;
    uint32_t indexCount;

    std::vector<Vertex> generateCubeVertices();
    std::vector<uint32_t> generateCubeIndices();
};
