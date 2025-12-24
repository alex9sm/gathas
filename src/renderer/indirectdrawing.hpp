#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include "vk_mem_alloc.h"
#include "gpubuffer.hpp"
#include "materialmanager.hpp"

class Mesh;
class CommandBuffer;

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct InstanceData {
	glm::mat4 modelMatrix;
};

struct IndirectDrawCommand {
	VkDrawIndexedIndirectCommand indirectCommand;
	const Mesh* mesh;
	uint32_t submeshIndex;
	uint32_t modelIndex;
	InstanceData instanceData;
};

/*
	MaterialBatch manages double-buffered GPU indirect draw buffers with staging.
	The drawCommands list holds all possible submesh draws for this material.
	Per frame, visible commands are copied to staging then transferred to GPU buffers.
*/
struct MaterialBatch {
	const MaterialManager::Material* material;
	std::vector<IndirectDrawCommand> drawCommands;

	GPUBuffer indirectBuffers[MAX_FRAMES_IN_FLIGHT];
	GPUBuffer stagingBuffers[MAX_FRAMES_IN_FLIGHT];
	uint32_t visibleCount[MAX_FRAMES_IN_FLIGHT];
	uint32_t maxCommands;
	bool buffersAllocated;
	VmaAllocator allocator;

	MaterialBatch() : material(nullptr), maxCommands(0), buffersAllocated(false), allocator(nullptr) {
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			visibleCount[i] = 0;
		}
	}
	~MaterialBatch() = default;

	MaterialBatch(const MaterialBatch&) = delete;
	MaterialBatch& operator=(const MaterialBatch&) = delete;

	MaterialBatch(MaterialBatch&&) noexcept = default;
	MaterialBatch& operator=(MaterialBatch&&) noexcept = default;

	void addDraw(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform);
	void addDrawWithOffsets(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform,
		uint32_t globalVertexOffset, uint32_t globalIndexOffset, uint32_t modelIndex = 0);

	void allocateBuffers(VmaAllocator allocator);
	void updateVisibleCommands(uint32_t frameIndex, const std::vector<VkDrawIndexedIndirectCommand>& commands);
	void recordBufferCopy(VkCommandBuffer cmd, uint32_t frameIndex);

	VkBuffer getIndirectBuffer(uint32_t frameIndex) const { return indirectBuffers[frameIndex].getBuffer(); }
	uint32_t getVisibleCount(uint32_t frameIndex) const { return visibleCount[frameIndex]; }

	void cleanup(VmaAllocator allocator);
};
