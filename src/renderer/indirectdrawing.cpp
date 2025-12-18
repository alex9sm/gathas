#include "indirectdrawing.hpp"
#include "mesh.hpp"
#include <iostream>
#include <stdexcept>

void MaterialBatch::addDraw(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform) {
	addDrawWithOffsets(mesh, submeshIndex, transform, 0, 0);
}

void MaterialBatch::addDrawWithOffsets(const Mesh* mesh, uint32_t submeshIndex, const glm::mat4& transform,
	uint32_t globalVertexOffset, uint32_t globalIndexOffset) {
	const SubMesh& submesh = mesh->getSubmesh(submeshIndex);

	IndirectDrawCommand cmd{};
	cmd.indirectCommand.indexCount = submesh.indexCount;
	cmd.indirectCommand.instanceCount = 1;
	cmd.indirectCommand.firstIndex = submesh.indexOffset + globalIndexOffset;
	cmd.indirectCommand.vertexOffset = static_cast<int32_t>(globalVertexOffset);
	cmd.indirectCommand.firstInstance = 0;
	cmd.mesh = mesh;
	cmd.submeshIndex = submeshIndex;
	cmd.instanceData.modelMatrix = transform;

	drawCommands.push_back(cmd);
}

void MaterialBatch::buildIndirectBuffer(VmaAllocator allocator) {
	if (drawCommands.empty()) {
		std::cerr << "MaterialBatch::buildIndirectBuffer: No draw commands to build buffer from" << std::endl;
		return;
	}

	std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
	indirectCommands.reserve(drawCommands.size());

	for (const auto& cmd : drawCommands) {
		indirectCommands.push_back(cmd.indirectCommand);
	}

	VkDeviceSize bufferSize = indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand);

	indirectBuffer.create(
		allocator,
		bufferSize,
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		indirectCommands.data(),
		nullptr
	);
}

void MaterialBatch::cleanup(VmaAllocator allocator) {
	indirectBuffer.destroy(allocator);
	drawCommands.clear();
}
