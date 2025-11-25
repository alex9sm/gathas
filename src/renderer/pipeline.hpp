#pragma once

#include <Vulkan/vulkan.h>
#include <string>
#include <vector>

class ShaderManager;
class Camera;
class MaterialManager;

class Pipeline {

public:
	Pipeline(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D swapChainExtent,
		VkFormat swapChainImageFormat, ShaderManager* shaderManager,
		const std::string& vertShaderName, const std::string& fragShaderName, Camera* camera,
		MaterialManager* materialManager);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	VkPipeline getPipeline() const { return graphicsPipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkRenderPass getRenderPass() const { return renderPass; }
	VkDescriptorSet getDescriptorSet(uint32_t frame) const { return descriptorSets[frame]; }
	VkImageView getDepthImageView() const { return depthImageView; }

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	Camera* camera;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkFormat depthFormat;

	VkDescriptorSetLayout materialDescriptorSetLayout;

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void createDepthResources(VkExtent2D swapChainExtent);
	void createGraphicsPipeline(VkExtent2D swapChainExtent, VkFormat swapChainImageFormat,
		ShaderManager* shaderManager, const std::string& vertShaderName,
		const std::string& fragShaderName);
	void createRenderPass(VkFormat swapChainImageFormat);

	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
};