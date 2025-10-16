#pragma once

#include <Vulkan/vulkan.h>
#include <string>
#include <vector>

class Pipeline {

public:
	Pipeline(VkDevice device, VkExtent2D swapChainExtent, VkFormat swapChainImageFormat);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	VkPipeline getPipeline() const { return graphicsPipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

private:
	VkDevice device;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	void createGraphicsPipeline(VkExtent2D swapChainExtent, VkFormat swapChainImageFormat);
	void createRenderPass(VkFormat swapChainImageFormat);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	std::vector<char> readFile(const std::string& filename);
};