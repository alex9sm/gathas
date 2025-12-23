#pragma once

#include "../imgui/imgui.h"
#include <string>

class Scene;
class DirectionalLight;
class PointLight;
class DirectionalLightPanel;
class PointLightPanel;
class CameraPanel;
class ScenePanel;

class RightPanel {
public:
    RightPanel(Scene* scene, DirectionalLight* light, PointLight* pointLight, DirectionalLightPanel* dirLightPanel, PointLightPanel* pointLightPanel, CameraPanel* cameraPanel, ScenePanel* scenePanel);
    ~RightPanel();

    RightPanel(const RightPanel&) = delete;
    RightPanel& operator=(const RightPanel&) = delete;

    void render();

private:
    Scene* scene;
    DirectionalLight* light;
    PointLight* pointLight;
    DirectionalLightPanel* dirLightPanel;
    PointLightPanel* pointLightPanel;
    CameraPanel* cameraPanel;
    ScenePanel* scenePanel;

    std::string selectedObject;
};
