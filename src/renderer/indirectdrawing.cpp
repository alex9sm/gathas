#include "indirectdrawing.hpp"
#include "mesh.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>

void MaterialBatch::addDraw(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform) {
	addDrawWithOffsets(mesh, submeshIndex, transform, 0, 0, 0);
}

void MaterialBatch::addDrawWithOffsets(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform,
	uint32_t globalVertexOffset, uint32_t globalIndexOffset, uint32_t modelIndex) {
	const SubMesh& submesh = mesh->getSubmesh(submeshIndex);

	IndirectDrawCommand cmd{};
	cmd.indirectCommand.indexCount = submesh.indexCount;
	cmd.indirectCommand.instanceCount = 1;
	cmd.indirectCommand.firstIndex = submesh.indexOffset + globalIndexOffset;
	cmd.indirectCommand.vertexOffset = static_cast<int32_t>(globalVertexOffset);
	cmd.indirectCommand.firstInstance = 0;
	cmd.mesh = mesh;
	cmd.submeshIndex = submeshIndex;
	cmd.modelIndex = modelIndex;
	cmd.instanceData.modelMatrix = transform;

	drawCommands.push_back(cmd);
}

void MaterialBatch::allocateBuffers(VmaAllocator alloc) {
	if (drawCommands.empty()) {
		return;
	}

	this->allocator = alloc;
	maxCommands = static_cast<uint32_t>(drawCommands.size());
	VkDeviceSize bufferSize = maxCommands * sizeof(VkDrawIndexedIndirectCommand);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		stagingBuffers[i].create(alloc, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			nullptr, nullptr);

		indirectBuffers[i].create(alloc, bufferSize,
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			nullptr, nullptr);

		visibleCount[i] = 0;
	}

	buffersAllocated = true;
}

void MaterialBatch::updateVisibleCommands(uint32_t frameIndex, const std::vector<VkDrawIndexedIndirectCommand>& commands) {
	if (!buffersAllocated) {
		visibleCount[frameIndex] = 0;
		return;
	}

	if (commands.empty()) {
		visibleCount[frameIndex] = 0;
		return;
	}

	uint32_t count = static_cast<uint32_t>(commands.size());
	if (count > maxCommands) {
		count = maxCommands;
	}

	void* mappedData;
	if (vmaMapMemory(allocator, stagingBuffers[frameIndex].getAllocation(), &mappedData) == VK_SUCCESS) {
		std::memcpy(mappedData, commands.data(), count * sizeof(VkDrawIndexedIndirectCommand));
		vmaUnmapMemory(allocator, stagingBuffers[frameIndex].getAllocation());
	}

	visibleCount[frameIndex] = count;
}

void MaterialBatch::recordBufferCopy(VkCommandBuffer cmd, uint32_t frameIndex) {
	if (!buffersAllocated || visibleCount[frameIndex] == 0) {
		return;
	}

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = visibleCount[frameIndex] * sizeof(VkDrawIndexedIndirectCommand);

	vkCmdCopyBuffer(cmd,
		stagingBuffers[frameIndex].getBuffer(),
		indirectBuffers[frameIndex].getBuffer(),
		1, &copyRegion);

	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = indirectBuffers[frameIndex].getBuffer();
	barrier.offset = 0;
	barrier.size = copyRegion.size;

	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);
}

void MaterialBatch::cleanup(VmaAllocator allocator) {
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		indirectBuffers[i].destroy(allocator);
		stagingBuffers[i].destroy(allocator);
		visibleCount[i] = 0;
	}
	drawCommands.clear();
	buffersAllocated = false;
	maxCommands = 0;
}
