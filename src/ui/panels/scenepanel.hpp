#pragma once

#include "../imgui/imgui.h"

class DirectionalLight;

class ScenePanel {
public:
    ScenePanel(DirectionalLight* light);
    ~ScenePanel();

    ScenePanel(const ScenePanel&) = delete;
    ScenePanel& operator=(const ScenePanel&) = delete;

    void render();
    void setOpen(bool open) { isOpen = open; }
    bool isWindowOpen() const { return isOpen; }

private:
    DirectionalLight* light;
    bool isOpen = false;
};
