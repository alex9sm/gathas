#include <vector>
#include "commandbuffer.hpp"
#include "vertex.hpp"

class GPUBuffer {

public:
	GPUBuffer(VkInstance& instance, CommandBuffer& commandBuffer, const std::vector<Vertex>& vertices, const std::vector<uint32_t> indicies);
	~GPUBuffer();

	

private:
	VkDevice& device;


};