#pragma once

#include "../imgui/imgui.h"
#include "../../core/scene.hpp"
#include <vector>
#include <string>

class BottomPanel {
public:
    BottomPanel(Scene* scene, const std::string& assetsPath);
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

    const float panelWidth = 200.0f;
    const float panelHeight = 600.0f;

    int selectedIndex = -1;

    void scanAssetFolders();
};
