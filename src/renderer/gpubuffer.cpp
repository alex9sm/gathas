#include "gpubuffer.hpp"
#include <stdexcept>
#include <iostream>

GPUBuffer::GPUBuffer() : buffer(nullptr), allocation(nullptr), size(0) {

}

GPUBuffer::~GPUBuffer() {}

void GPUBuffer::create(VmaAllocator allocator, VkDeviceSize bufferSize,
    VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
    const void* data, CommandBuffer* commandBuffer) {

    size = bufferSize;

    bool needsStaging = (data != nullptr) &&
        (memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) &&
        (commandBuffer != nullptr);

    if (needsStaging) {
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        VkBufferCreateInfo stagingBufferInfo{};
        stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferInfo.size = size;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        if (vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocInfo,
            &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create staging buffer");
        }

        void* mappedData;
        vmaMapMemory(allocator, stagingAllocation, &mappedData);
        memcpy(mappedData, data, size);
        vmaUnmapMemory(allocator, stagingAllocation);

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
            &buffer, &allocation, nullptr) != VK_SUCCESS) {
            vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
            throw std::runtime_error("failed to create GPU buffer");
        }

        copyBuffer(commandBuffer, stagingBuffer, buffer, size);

        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

    }
    else {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
            &buffer, &allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer");
        }

        if (data != nullptr) {
            void* mappedData;
            vmaMapMemory(allocator, allocation, &mappedData);
            memcpy(mappedData, data, size);
            vmaUnmapMemory(allocator, allocation);
        }
    }
}

void GPUBuffer::copyBuffer(CommandBuffer* commandBuffer, VkBuffer srcBuffer,
    VkBuffer dstBuffer, VkDeviceSize bufferSize) {

    VkCommandBuffer cmdBuffer = commandBuffer->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    commandBuffer->endSingleTimeCommands(cmdBuffer);
}

void GPUBuffer::destroy(VmaAllocator allocator) {
    if (buffer != nullptr) {
        vmaDestroyBuffer(allocator, buffer, allocation);
        buffer = nullptr;
        allocation = nullptr;
        size = 0;
    }
}