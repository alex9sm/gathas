#pragma once

#include <Vulkan/vulkan.h>
#include <string>
#include <vector>

class ShaderManager;

class Pipeline {

public:
	Pipeline(VkDevice device, VkExtent2D swapChainExtent, VkFormat swapChainImageFormat,
		     ShaderManager* shaderManager, const std::string& vertShaderName,
		     const std::string& fragShaderName);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	VkPipeline getPipeline() const { return graphicsPipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkRenderPass getRenderPass() const { return renderPass; }
 
private:
	VkDevice device;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	void createGraphicsPipeline(VkExtent2D swapChainExtent, VkFormat swapChainImageFormat,
		                        ShaderManager* shaderManager, const std::string& vertShaderName,
		                        const std::string& fragShaderName);
	void createRenderPass(VkFormat swapChainImageFormat);
};