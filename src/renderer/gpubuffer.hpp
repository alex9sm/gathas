#pragma once

#include <vector>
#include "commandbuffer.hpp"
#include "vertex.hpp"
#include "vk_mem_alloc.h"

class GPUBuffer {

public:
	GPUBuffer();
	~GPUBuffer();

	GPUBuffer(const GPUBuffer&) = delete;
	GPUBuffer& operator=(const GPUBuffer&) = delete;

	void create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
		VmaMemoryUsage memoryUsage, const void* data = nullptr,
		CommandBuffer* commandBuffer = nullptr);

	void destroy(VmaAllocator allocator);

	VkBuffer getBuffer() const { return buffer; }
	VkDeviceSize getSize() const { return size; }

private:
	VkDevice device;
	VkBuffer buffer;
	VmaAllocation allocation;
	VkDeviceSize size;

	void copyBuffer(CommandBuffer* commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

};