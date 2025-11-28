#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

const uint32_t W = 1600;
const uint32_t H = 900;

class GatWindow {

public:
	GatWindow();
	~GatWindow();

	void initWindow();
	bool shouldClose();
	void pollEvents();
	GLFWwindow* getWindow() { return window; }
	void setFramebufferResizeCallback(GLFWframebuffersizefun callback) {
		glfwSetFramebufferSizeCallback(window, callback);
	}

	GatWindow(const GatWindow&) = delete;
	GatWindow& operator=(const GatWindow&) = delete;
	
private:
	GLFWwindow* window;

};