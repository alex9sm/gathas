#pragma once

#include "../imgui/imgui.h"
#include <string>

class Scene;
class DirectionalLight;
class DirectionalLightPanel;
class CameraPanel;

class RightPanel {
public:
    RightPanel(Scene* scene, DirectionalLight* light, DirectionalLightPanel* dirLightPanel, CameraPanel* cameraPanel);
    ~RightPanel();

    RightPanel(const RightPanel&) = delete;
    RightPanel& operator=(const RightPanel&) = delete;

    void render();

private:
    Scene* scene;
    DirectionalLight* light;
    DirectionalLightPanel* dirLightPanel;
    CameraPanel* cameraPanel;

    std::string selectedObject;
};
