#pragma once

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

	
private:
	GLFWwindow* window;

};