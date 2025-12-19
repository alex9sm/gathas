#pragma once

#include "../imgui/imgui.h"

class DirectionalLight;

class DirectionalLightPanel {
public:
    DirectionalLightPanel(DirectionalLight* light);
    ~DirectionalLightPanel();

    DirectionalLightPanel(const DirectionalLightPanel&) = delete;
    DirectionalLightPanel& operator=(const DirectionalLightPanel&) = delete;

    void render();
    void setOpen(bool open) { isOpen = open; }
    bool isWindowOpen() const { return isOpen; }

private:
    DirectionalLight* light;
    bool isOpen = false;
};
