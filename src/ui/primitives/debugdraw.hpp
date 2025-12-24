#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "vk_mem_alloc.h"

class CommandBuffer;

struct DebugVertex {
    glm::vec3 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(DebugVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(DebugVertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(DebugVertex, color);

        return attributeDescriptions;
    }
};

class DebugDraw {
public:
    DebugDraw();
    ~DebugDraw();

    DebugDraw(const DebugDraw&) = delete;
    DebugDraw& operator=(const DebugDraw&) = delete;

    void init(VkDevice device, VmaAllocator allocator, CommandBuffer* commandBuffer);
    void cleanup();

    void drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color);
    void drawAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
    void drawBox(const glm::vec3& center, const glm::vec3& halfExtents, const glm::vec3& color);
    void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments = 16);
    void drawGrid(const glm::vec3& center, float size, int divisions, const glm::vec3& color);
    void drawAxis(const glm::vec3& origin, float length);

    void clear();
    void upload();
    void bind(VkCommandBuffer cmdBuffer);
    void draw(VkCommandBuffer cmdBuffer);

    bool hasLines() const { return !vertices.empty(); }
    size_t getVertexCount() const { return vertices.size(); }

private:
    VkDevice device;
    VmaAllocator allocator;
    CommandBuffer* commandBuffer;

    std::vector<DebugVertex> vertices;

    VkBuffer vertexBuffer;
    VmaAllocation vertexAllocation;
    VkDeviceSize bufferSize;
    bool bufferCreated;

    static constexpr size_t INITIAL_BUFFER_SIZE = 1024 * sizeof(DebugVertex);

    void createOrResizeBuffer();
};
