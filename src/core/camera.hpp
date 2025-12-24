#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../renderer/gpubuffer.hpp"
#include "vk_mem_alloc.h"

// forward declaration
class ImGuiLayer;

struct CameraUBO {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 invProj;
};

class Camera {
public:
    Camera(GLFWwindow* window, VmaAllocator allocator);
    ~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void update(float deltaTime);
    void updateUniformBuffer(VmaAllocator allocator, uint32_t currentFrame);
    void destroy(VmaAllocator allocator);

    VkBuffer getBuffer(uint32_t frame) const { return uniformBuffers[frame].getBuffer(); }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(float yaw, float pitch) { this->yaw = yaw; this->pitch = pitch; }
    glm::vec3 getPosition() const { return position; }

    float* getSpeedPtr() { return &movementSpeed; }
    glm::mat4 getViewProjectionMatrix() const;
    glm::mat4 getCullingViewProjectionMatrix() const;

    void setDebugCullingFov(float fov) { debugCullingFov = fov; }
    float getDebugCullingFov() const { return debugCullingFov; }
    void setUseDebugCullingFov(bool use) { useDebugCullingFov = use; }
    bool getUseDebugCullingFov() const { return useDebugCullingFov; }

    static const int MAX_FRAMES_IN_FLIGHT = 2;

    void setupInputCallbacks(GLFWwindow* window);
    bool isCursorCaptured() const { return cursorCaptured; }

    void setImGuiLayer(ImGuiLayer* layer);

private:
    GLFWwindow* window;

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float fov;

    float movementSpeed;
    float mouseSensitivity;

    float debugCullingFov = 45.0f;
    bool useDebugCullingFov = false;

    bool firstMouse;
    double lastX;
    double lastY;

    GPUBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];

    void processKeyboard(float deltaTime);
    void processMouseMovement();
    void updateCameraVectors();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    bool cursorCaptured = false;
    void handleCursorCapture();
    void handleCursorRelease();
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    ImGuiLayer* imguiLayer;
};