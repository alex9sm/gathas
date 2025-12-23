#pragma once

#include "../imgui/imgui.h"

class PointLight;

class PointLightPanel {
public:
    PointLightPanel(PointLight* light);
    ~PointLightPanel();

    PointLightPanel(const PointLightPanel&) = delete;
    PointLightPanel& operator=(const PointLightPanel&) = delete;

    void render();
    void setOpen(bool open) { isOpen = open; }
    bool isWindowOpen() const { return isOpen; }

private:
    PointLight* light;
    bool isOpen = false;
};
