#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include "window.hpp"
#include <Vulkan/vulkan.h>

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    GatWindow window;
    VkInstance instance;

    void mainLoop();
    void initVulkan();
    void createInstance();
    void cleanup();
};