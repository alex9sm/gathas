#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include "vk_mem_alloc.h"
#include "gpubuffer.hpp"
#include "materialmanager.hpp"

class Mesh;

struct InstanceData {
	glm::mat4 modelMatrix;
};

struct IndirectDrawCommand {
	VkDrawIndexedIndirectCommand indirectCommand;
	const Mesh* mesh;
	uint32_t submeshIndex;
	InstanceData instanceData;
};

struct MaterialBatch {
	const MaterialManager::Material* material;
	std::vector<IndirectDrawCommand> drawCommands;
	GPUBuffer indirectBuffer;

	MaterialBatch() : material(nullptr) {}
	~MaterialBatch() = default;

	MaterialBatch(const MaterialBatch&) = delete;
	MaterialBatch& operator=(const MaterialBatch&) = delete;

	MaterialBatch(MaterialBatch&&) noexcept = default;
	MaterialBatch& operator=(MaterialBatch&&) noexcept = default;

	void addDraw(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform);
	void addDrawWithOffsets(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform,
		uint32_t globalVertexOffset, uint32_t globalIndexOffset);
	void buildIndirectBuffer(VmaAllocator allocator);
	void cleanup(VmaAllocator allocator);
};
