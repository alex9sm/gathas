#include "window.hpp"

GatWindow::GatWindow() : window(nullptr) {

}

GatWindow::~GatWindow() {
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}

void GatWindow::initWindow() {

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//glfwGetPrimaryMonitor() - fullscreen
	window = glfwCreateWindow(W, H, "Gathas", nullptr, nullptr);

	if (!window) {
		glfwTerminate();
	}
}

bool GatWindow::shouldClose() {
	return glfwWindowShouldClose(window);
}

void GatWindow::pollEvents() {
	glfwPollEvents();
}