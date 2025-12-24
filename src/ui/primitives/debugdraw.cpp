#include "debugdraw.hpp"
#include "../../renderer/commandbuffer.hpp"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DebugDraw::DebugDraw()
    : device(VK_NULL_HANDLE), allocator(VK_NULL_HANDLE), commandBuffer(nullptr),
      vertexBuffer(VK_NULL_HANDLE), vertexAllocation(VK_NULL_HANDLE),
      bufferSize(0), bufferCreated(false) {
}

DebugDraw::~DebugDraw() {
}

void DebugDraw::init(VkDevice device, VmaAllocator allocator, CommandBuffer* commandBuffer) {
    this->device = device;
    this->allocator = allocator;
    this->commandBuffer = commandBuffer;

    vertices.reserve(1024);

    std::cout << "debug draw initialized" << std::endl;
}

void DebugDraw::cleanup() {
    if (bufferCreated && vertexBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
        vertexBuffer = VK_NULL_HANDLE;
        vertexAllocation = VK_NULL_HANDLE;
        bufferCreated = false;
    }
    vertices.clear();
}

void DebugDraw::drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    vertices.push_back({ start, color });
    vertices.push_back({ end, color });
}

void DebugDraw::drawAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
    // bottom face
    drawLine(glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z), color);
    drawLine(glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, min.y, max.z), color);
    drawLine(glm::vec3(max.x, min.y, max.z), glm::vec3(min.x, min.y, max.z), color);
    drawLine(glm::vec3(min.x, min.y, max.z), glm::vec3(min.x, min.y, min.z), color);

    // top face
    drawLine(glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z), color);
    drawLine(glm::vec3(max.x, max.y, min.z), glm::vec3(max.x, max.y, max.z), color);
    drawLine(glm::vec3(max.x, max.y, max.z), glm::vec3(min.x, max.y, max.z), color);
    drawLine(glm::vec3(min.x, max.y, max.z), glm::vec3(min.x, max.y, min.z), color);

    // vertical edges
    drawLine(glm::vec3(min.x, min.y, min.z), glm::vec3(min.x, max.y, min.z), color);
    drawLine(glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, max.y, min.z), color);
    drawLine(glm::vec3(max.x, min.y, max.z), glm::vec3(max.x, max.y, max.z), color);
    drawLine(glm::vec3(min.x, min.y, max.z), glm::vec3(min.x, max.y, max.z), color);
}

void DebugDraw::drawBox(const glm::vec3& center, const glm::vec3& halfExtents, const glm::vec3& color) {
    glm::vec3 min = center - halfExtents;
    glm::vec3 max = center + halfExtents;
    drawAABB(min, max, color);
}

void DebugDraw::drawSphere(const glm::vec3& center, float radius, const glm::vec3& color, int segments) {
    float angleStep = 2.0f * static_cast<float>(M_PI) / static_cast<float>(segments);

    // XY circle
    for (int i = 0; i < segments; i++) {
        float angle1 = static_cast<float>(i) * angleStep;
        float angle2 = static_cast<float>(i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(std::cos(angle1) * radius, std::sin(angle1) * radius, 0.0f);
        glm::vec3 p2 = center + glm::vec3(std::cos(angle2) * radius, std::sin(angle2) * radius, 0.0f);
        drawLine(p1, p2, color);
    }

    // XZ circle
    for (int i = 0; i < segments; i++) {
        float angle1 = static_cast<float>(i) * angleStep;
        float angle2 = static_cast<float>(i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(std::cos(angle1) * radius, 0.0f, std::sin(angle1) * radius);
        glm::vec3 p2 = center + glm::vec3(std::cos(angle2) * radius, 0.0f, std::sin(angle2) * radius);
        drawLine(p1, p2, color);
    }

    // YZ circle
    for (int i = 0; i < segments; i++) {
        float angle1 = static_cast<float>(i) * angleStep;
        float angle2 = static_cast<float>(i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(0.0f, std::cos(angle1) * radius, std::sin(angle1) * radius);
        glm::vec3 p2 = center + glm::vec3(0.0f, std::cos(angle2) * radius, std::sin(angle2) * radius);
        drawLine(p1, p2, color);
    }
}

void DebugDraw::drawGrid(const glm::vec3& center, float size, int divisions, const glm::vec3& color) {
    float halfSize = size / 2.0f;
    float step = size / static_cast<float>(divisions);

    for (int i = 0; i <= divisions; i++) {
        float offset = -halfSize + static_cast<float>(i) * step;
        // lines along X
        drawLine(center + glm::vec3(-halfSize, 0.0f, offset),
                 center + glm::vec3(halfSize, 0.0f, offset), color);
        // lines along Z
        drawLine(center + glm::vec3(offset, 0.0f, -halfSize),
                 center + glm::vec3(offset, 0.0f, halfSize), color);
    }
}

void DebugDraw::drawAxis(const glm::vec3& origin, float length) {
    drawLine(origin, origin + glm::vec3(length, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // X - red
    drawLine(origin, origin + glm::vec3(0.0f, length, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y - green
    drawLine(origin, origin + glm::vec3(0.0f, 0.0f, length), glm::vec3(0.0f, 0.0f, 1.0f)); // Z - blue
}

void DebugDraw::clear() {
    vertices.clear();
}

void DebugDraw::createOrResizeBuffer() {
    VkDeviceSize requiredSize = vertices.size() * sizeof(DebugVertex);
    if (requiredSize == 0) {
        requiredSize = INITIAL_BUFFER_SIZE;
    }

    // only recreate if we need more space
    if (bufferCreated && bufferSize >= requiredSize) {
        return;
    }

    // destroy old buffer
    if (bufferCreated && vertexBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
    }

    // create new buffer with extra capacity
    bufferSize = requiredSize * 2;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &vertexBuffer, &vertexAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug vertex buffer");
    }

    bufferCreated = true;
}

void DebugDraw::upload() {
    if (vertices.empty()) {
        return;
    }

    createOrResizeBuffer();

    void* data;
    vmaMapMemory(allocator, vertexAllocation, &data);
    memcpy(data, vertices.data(), vertices.size() * sizeof(DebugVertex));
    vmaUnmapMemory(allocator, vertexAllocation);
}

void DebugDraw::bind(VkCommandBuffer cmdBuffer) {
    if (!bufferCreated || vertices.empty()) {
        return;
    }

    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
}

void DebugDraw::draw(VkCommandBuffer cmdBuffer) {
    if (!bufferCreated || vertices.empty()) {
        return;
    }

    vkCmdDraw(cmdBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
}
