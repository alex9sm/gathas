#pragma once

#include "../imgui/imgui.h"
#include "../../core/scene.hpp"
#include <vector>
#include <string>

class DirectionalLight;
class PointLight;

class BottomPanel {
public:
    BottomPanel(Scene* scene, const std::string& assetsPath, DirectionalLight* dirLight, PointLight* pointLight);
    ~BottomPanel();

    BottomPanel(const BottomPanel&) = delete;
    BottomPanel& operator=(const BottomPanel&) = delete;

    void render();
    float getWidth() const { return panelWidth; }
    float getHeight() const { return panelHeight; }

private:
    Scene* scene;
    std::string assetsPath;
    std::vector<std::string> assetFolders;
    DirectionalLight* dirLight;
    PointLight* pointLight;

    const float panelWidth = 200.0f;
    const float panelHeight = 600.0f;

    int selectedModelIndex = -1;
    int selectedActorIndex = -1;

    void scanAssetFolders();
};
