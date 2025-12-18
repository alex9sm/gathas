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

	GPUBuffer(GPUBuffer&& other) noexcept;
	GPUBuffer& operator=(GPUBuffer&& other) noexcept;

	void create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
		VmaMemoryUsage memoryUsage, const void* data = nullptr,
		CommandBuffer* commandBuffer = nullptr);

	void destroy(VmaAllocator allocator);

	VkBuffer getBuffer() const { return buffer; }
	VkDeviceSize getSize() const { return size; }
	VmaAllocation getAllocation() const { return allocation; }

private:
	VkDevice device;
	VkBuffer buffer;
	VmaAllocation allocation;
	VkDeviceSize size;

	void copyBuffer(CommandBuffer* commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

};