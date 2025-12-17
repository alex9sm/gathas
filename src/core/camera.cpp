#include "camera.hpp"
#include "../ui/imguilayer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// STRUCT!!
struct WindowUserData {
    void* app;
    Camera* camera;
};

Camera::Camera(GLFWwindow* window, VmaAllocator allocator)
    : window(window),
    position(0.0f, 0.0f, 0.0f),
    worldUp(0.0f, 1.0f, 0.0f),
    yaw(-90.0f),
    pitch(0.0f),
    fov(90.0f),
    movementSpeed(500.0f),
    mouseSensitivity(0.15f),
    firstMouse(true),
    lastX(0.0),
    lastY(0.0),
    imguiLayer(nullptr),
    cursorCaptured(false) {

    updateCameraVectors();

    VkDeviceSize bufferSize = sizeof(CameraUBO);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers[i].create(allocator, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
}

Camera::~Camera() {
}

void Camera::setImGuiLayer(ImGuiLayer* layer) {
    imguiLayer = layer;
}

void Camera::update(float deltaTime) {
    // process camera input only if cursor is captured
    if (cursorCaptured) {
        processKeyboard(deltaTime);
        processMouseMovement();
    }

    updateCameraVectors();
}

void Camera::updateUniformBuffer(VmaAllocator allocator, uint32_t currentFrame) {
    CameraUBO ubo{};
    ubo.view = getViewMatrix();
    ubo.proj = getProjectionMatrix();
    ubo.proj[1][1] *= 1;

    ubo.invView = glm::inverse(ubo.view);
    ubo.invProj = glm::inverse(ubo.proj);

    void* data;
    vmaMapMemory(allocator, uniformBuffers[currentFrame].getAllocation(), &data);
    memcpy(data, &ubo, sizeof(CameraUBO));
    vmaUnmapMemory(allocator, uniformBuffers[currentFrame].getAllocation());
}

void Camera::destroy(VmaAllocator allocator) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers[i].destroy(allocator);
    }
}

void Camera::processKeyboard(float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += worldUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        position -= worldUp * velocity;
}

void Camera::processMouseMovement() {
    if (!cursorCaptured) {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += static_cast<float>(xoffset);
    pitch += static_cast<float>(yoffset);

    // constrain pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    if (height == 0) {
        height = 1;
    }

    float aspect = static_cast<float>(width) / static_cast<float>(height);

    return glm::perspective(glm::radians(fov), aspect, 0.1f, 10000.0f);
}

void Camera::setupInputCallbacks(GLFWwindow* win) {
    window = win;
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    cursorCaptured = false;
}

void Camera::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto userData = reinterpret_cast<WindowUserData*>(glfwGetWindowUserPointer(window));
    Camera* camera = userData->camera;

    // prevent capturing if clicking on imgui
    if (camera->imguiLayer && camera->imguiLayer->wantCaptureMouse()) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !camera->cursorCaptured) {
        camera->handleCursorCapture();
    }
}

void Camera::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto userData = reinterpret_cast<WindowUserData*>(glfwGetWindowUserPointer(window));
    Camera* camera = userData->camera;

    // don't release if imgui wants the keyboard
    if (camera->imguiLayer && camera->imguiLayer->wantCaptureKeyboard()) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && camera->cursorCaptured) {
        camera->handleCursorRelease();
    }
}

void Camera::handleCursorCapture() {
    cursorCaptured = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    firstMouse = true;
}

void Camera::handleCursorRelease() {
    cursorCaptured = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}