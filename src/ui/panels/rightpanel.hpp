#pragma once

#include "../imgui/imgui.h"
#include <string>

class Scene;
class DirectionalLight;
class DirectionalLightPanel;

class RightPanel {
public:
    RightPanel(Scene* scene, DirectionalLight* light, DirectionalLightPanel* dirLightPanel);
    ~RightPanel();

    RightPanel(const RightPanel&) = delete;
    RightPanel& operator=(const RightPanel&) = delete;

    void render();

private:
    Scene* scene;
    DirectionalLight* light;
    DirectionalLightPanel* dirLightPanel;

    std::string selectedObject;
};
