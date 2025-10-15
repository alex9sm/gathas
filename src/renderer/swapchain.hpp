#pragma once

#include <Vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

struct SwapChainSupport {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain {

public:
	SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, GLFWwindow* window);
	~SwapChain();

	SwapChain(const SwapChain&) = delete;
	SwapChain& operator = (const SwapChain&) = delete;

	VkSwapchainKHR getSwapChain() const { return swapChain; }
	VkFormat getImageFormat() const { return swapChainImageFormat; }
	VkExtent2D getExtent() const { return swapChainExtent; }
	const std::vector<VkImageView>& getImageViews() const { return swapChainImageViews; }
	size_t getImageCount() const { return swapChainImages.size(); }

private:

	//vulkan handles
	VkDevice device;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	void createSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window);
	void createImageViews();

	//query swapchain support and pick settings
	SwapChainSupport querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

};