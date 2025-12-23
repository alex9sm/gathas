#pragma once

#include <Vulkan/vulkan.h>
#include <string>
#include <vector>

class ShaderManager;
class Camera;
class MaterialManager;
class GBuffer;
class DirectionalLight;
class PointLight;

class Pipeline {

public:
	Pipeline(VkDevice device, VkPhysicalDevice physicalDevice);
	~Pipeline();

	void initialize(VkExtent2D swapChainExtent, VkFormat swapChainImageFormat,
		ShaderManager* shaderManager, const std::string& vertShaderName,
		const std::string& fragShaderName, Camera* camera, MaterialManager* materialManager,
		GBuffer* gbuffer, const std::vector<VkImageView>& swapChainImageViews,
		DirectionalLight* light, PointLight* pointLight);

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	VkPipeline getGeometryPipeline() const { return geometryPipeline; }
	VkPipelineLayout getGeometryPipelineLayout() const { return pipelineLayout; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkPipeline getLightingPipeline() const { return lightingPipeline; }
	VkPipelineLayout getLightingPipelineLayout() const { return lightingPipelineLayout; }
	VkPipeline getForwardPipeline() const { return forwardPipeline; }
	VkPipelineLayout getForwardPipelineLayout() const { return forwardPipelineLayout; }
	VkRenderPass getGeometryRenderPass() const { return geometryRenderPass; }
	VkRenderPass getLightingRenderPass() const { return lightingRenderPass; }
	VkRenderPass getForwardRenderPass() const { return forwardRenderPass; }
	VkRenderPass getImGuiRenderPass() const { return imguiRenderPass; }
	VkFramebuffer getGeometryFramebuffer(uint32_t index) const { return geometryFramebuffers[index]; }
	VkFramebuffer getLightingFramebuffer(uint32_t index) const { return lightingFramebuffers[index]; }
	VkFramebuffer getForwardFramebuffer(uint32_t index) const { return forwardFramebuffers[index]; }
	VkFramebuffer getImGuiFramebuffer(uint32_t index) const { return imguiFramebuffers[index]; }
	const std::vector<VkFramebuffer>& getGeometryFramebuffers() const { return geometryFramebuffers; }
	const std::vector<VkFramebuffer>& getLightingFramebuffers() const { return lightingFramebuffers; }
	const std::vector<VkFramebuffer>& getForwardFramebuffers() const { return forwardFramebuffers; }
	const std::vector<VkFramebuffer>& getImGuiFramebuffers() const { return imguiFramebuffers; }
	VkDescriptorSet getDescriptorSet(uint32_t frame) const { return descriptorSets[frame]; }
	VkImageView getDepthImageView() const { return depthImageView; }

	void createGeometryFramebuffers(GBuffer* gbuffer, uint32_t swapChainImageCount);
	void createLightingFramebuffers(const std::vector<VkImageView>& swapChainImageViews);
	void createImGuiFramebuffers(const std::vector<VkImageView>& swapChainImageViews);
	void createForwardFramebuffers(const std::vector<VkImageView>& swapChainImageViews);
	void createLightingPipeline(ShaderManager* shaderManager, GBuffer* gbuffer, DirectionalLight* light, PointLight* pointLight);
	void createForwardPipeline(ShaderManager* shaderManager, DirectionalLight* light);

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkPipeline geometryPipeline;
	VkPipelineLayout pipelineLayout;
	VkPipeline lightingPipeline;
	VkPipelineLayout lightingPipelineLayout;
	VkPipeline forwardPipeline;
	VkPipelineLayout forwardPipelineLayout;
	VkDescriptorSetLayout lightingDescriptorSetLayout;
	VkRenderPass geometryRenderPass;
	VkRenderPass lightingRenderPass;
	VkRenderPass forwardRenderPass;
	VkRenderPass imguiRenderPass;
	std::vector<VkFramebuffer> geometryFramebuffers;
	std::vector<VkFramebuffer> lightingFramebuffers;
	std::vector<VkFramebuffer> forwardFramebuffers;
	std::vector<VkFramebuffer> imguiFramebuffers;
	VkExtent2D swapChainExtent;
	VkFormat swapChainImageFormat;

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
	void createGeometryPipeline(ShaderManager* shaderManager);
	void createGeometryRenderPass();
	void createLightingRenderPass(VkFormat swapChainImageFormat);
	void createForwardRenderPass(VkFormat swapChainImageFormat);
	void createImGuiRenderPass(VkFormat swapChainImageFormat);

	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
};