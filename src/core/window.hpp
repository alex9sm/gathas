#pragma once

#include <GLFW\glfw3.h>

const uint32_t W = 1600;
const uint32_t H = 900;

class GatWindow {

public:
	void initWindow();
	
private:
	GLFWwindow* window;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(W, H, "Gathas", nullptr, nullptr);
	}

};