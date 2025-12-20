#pragma once

#include "../imgui/imgui.h"

class Camera;

class CameraPanel {
public:
    CameraPanel(Camera* camera);
    ~CameraPanel();

    CameraPanel(const CameraPanel&) = delete;
    CameraPanel& operator=(const CameraPanel&) = delete;

    void render();
    void setOpen(bool open) { isOpen = open; }
    bool isWindowOpen() const { return isOpen; }

private:
    Camera* camera;
    bool isOpen = false;
};
