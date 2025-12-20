#define VMA_IMPLEMENTATION
#include "application.hpp"
#include "scene.hpp"
#include "../renderer/swapchain.hpp"
#include "../renderer/pipeline.hpp"
#include "../renderer/shadermanager.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>

Application::Application() {
    window.initWindow();
    window.setFramebufferResizeCallback(framebufferResizeCallback);
    windowUserData.app = this;
    windowUserData.camera = nullptr;
    glfwSetWindowUserPointer(window.getWindow(), &windowUserData);
}

Application::~Application() {
    cleanup();
}

void Application::run() {
    initVulkan();
    mainLoop();
}

void Application::mainLoop() {
    while (!window.shouldClose()) {
        window.pollEvents();

        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;

        camera->update(deltaTime);
        drawFrame();
    }
    vkDeviceWaitIdle(device);
}

void Application::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createShaderManager();
    createCamera();
    createCommandBuffer();
    createSwapChain();
    createTextureManager();
    createMaterialManager();
    createGBuffer();
    createDirectionalLight();
    createPipeline();
    createScene();
    createImGuiLayer();

    camera->setImGuiLayer(imguiLayer.get());
    lastFrameTime = static_cast<float>(glfwGetTime());
}

void Application::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gathas";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance");
    }
}

void Application::createSurface() {
    glfwCreateWindowSurface(instance, window.getWindow(), nullptr, &surface);
}

void Application::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find gpus with vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == nullptr) {
        throw std::runtime_error("failed to find a suitable gpu");
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    std::cout << "selected gpu: " << deviceProperties.deviceName << std::endl;
}

bool Application::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete();
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void Application::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    //specify device specific features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // create the logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    std::cout << "device created" << std::endl;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Application::createShaderManager() {
    shaderManager = std::make_unique<ShaderManager>(device);
}

void Application::createCamera() {
    camera = std::make_unique<Camera>(window.getWindow(), allocator);
    windowUserData.camera = camera.get();
    camera->setupInputCallbacks(window.getWindow());
}

void Application::createSwapChain() {
    swapChain = std::make_unique<SwapChain>(physicalDevice, device, surface, window.getWindow());
}

void Application::createTextureManager() {
    textureManager = std::make_unique<TextureManager>(device, physicalDevice, allocator, commandBuffer.get());
}

void Application::createMaterialManager() {
    materialManager = std::make_unique<MaterialManager>(device, textureManager.get());
}

void Application::createGBuffer() {
    gbuffer = std::make_unique<GBuffer>(device, physicalDevice, allocator, swapChain->getExtent());
}

void Application::createDirectionalLight() {
    directionalLight = std::make_unique<DirectionalLight>(device, allocator);
}

void Application::createPipeline() {
    pipeline = std::make_unique<Pipeline>(device, physicalDevice);
    pipeline->initialize(swapChain->getExtent(), swapChain->getImageFormat(),
        shaderManager.get(), "geometry_vert.spv", "geometry_frag.spv",
        camera.get(), materialManager.get(), gbuffer.get(), swapChain->getImageViews(),
        directionalLight.get());
}

void Application::createCommandBuffer() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    commandBuffer = std::make_unique<CommandBuffer>(device, indices.graphicsFamily.value(), graphicsQueue);
}

void Application::createImGuiLayer() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    imguiLayer = std::make_unique<ImGuiLayer>();
    imguiLayer->init(window.getWindow(), instance, physicalDevice, device,
        indices.graphicsFamily.value(), graphicsQueue,
        pipeline->getImGuiRenderPass(),
        static_cast<uint32_t>(swapChain->getImageCount()),
        scene.get(), directionalLight.get(), camera.get());
}

void Application::createScene() {
    scene = std::make_unique<Scene>(allocator, commandBuffer.get(),
        materialManager.get(), textureManager.get());
}

void Application::drawFrame() {
    size_t maxFrames = commandBuffer->getMaxFramesInFlight();

    vkWaitForFences(device, 1, &commandBuffer->getInFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain->getSwapChain(), UINT64_MAX,
        commandBuffer->getImageAvailableSemaphore(currentFrame),
        nullptr, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(device, 1, &commandBuffer->getInFlightFence(currentFrame));

    // update camera uniform buffer
    camera->updateUniformBuffer(allocator, currentFrame);

    // update light uniform buffer
    directionalLight->updateUniformBuffer(allocator, currentFrame, camera->getPosition());

    VkCommandBuffer cmdBuffer = commandBuffer->getCommandBuffer(currentFrame);
    vkResetCommandBuffer(cmdBuffer, 0);

    // fps and frame time
    float currentFrameTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;
    imguiLayer->beginFrame();
    imguiLayer->endFrame(deltaTime);

    glm::mat4 viewProj = camera->getViewProjectionMatrix();

    commandBuffer->recordFrame(cmdBuffer, imageIndex, swapChain->getExtent(),
        pipeline->getGeometryRenderPass(),
        pipeline->getGeometryFramebuffers(),
        pipeline->getGeometryPipeline(), pipeline->getPipelineLayout(),
        pipeline->getDescriptorSet(currentFrame), materialManager.get(), scene.get(),
        pipeline->getLightingRenderPass(),
        pipeline->getLightingFramebuffers(),
        pipeline->getLightingPipeline(), pipeline->getLightingPipelineLayout(),
        gbuffer->getDescriptorSet(currentFrame),
        directionalLight->getDescriptorSet(currentFrame),
        pipeline->getForwardRenderPass(),
        pipeline->getForwardFramebuffers(),
        pipeline->getForwardPipeline(), pipeline->getForwardPipelineLayout(),
        viewProj,
        pipeline->getImGuiRenderPass(),
        pipeline->getImGuiFramebuffers(),
        imguiLayer.get());

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { commandBuffer->getImageAvailableSemaphore(currentFrame) };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkSemaphore signalSemaphores[] = { commandBuffer->getRenderFinishedSemaphore(currentFrame) };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, commandBuffer->getInFlightFence(currentFrame)) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain->getSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % maxFrames;
}

void Application::cleanup() {
    scene.reset();

    if (imguiLayer) {
        imguiLayer->cleanup();
    }
    imguiLayer.reset();

    //cleanup material and texture managers before commandBuffer
    if (materialManager) {
        materialManager->cleanup();
    }
    materialManager.reset();

    if (textureManager) {
        textureManager->cleanup();
    }
    textureManager.reset();

    commandBuffer.reset();
    pipeline.reset();

    if (directionalLight) {
        directionalLight->cleanup(allocator);
    }
    directionalLight.reset();

    if (gbuffer) {
        gbuffer->cleanup();
    }
    gbuffer.reset();

    swapChain.reset();
    if (camera) {
        camera->destroy(allocator);
    }
    camera.reset();
    if (shaderManager) {
        shaderManager->cleanup();
    }
    shaderManager.reset();
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto userData = reinterpret_cast<WindowUserData*>(glfwGetWindowUserPointer(window));
    userData->app->framebufferResized = true;
}

void Application::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.getWindow(), &width, &height);

    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.getWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    swapChain.reset();
    pipeline.reset();

    createSwapChain();

    gbuffer->recreate(swapChain->getExtent(), VK_NULL_HANDLE);

    createPipeline();

    gbuffer->updateDescriptorSets(pipeline->getDepthImageView());

    if (imguiLayer) {
        imguiLayer->cleanup();
    }
    createImGuiLayer();

    camera->setImGuiLayer(imguiLayer.get());
}