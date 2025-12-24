#pragma once

#include "../imgui/imgui.h"

class LeftPanel {
public:
    LeftPanel();
    ~LeftPanel();

    LeftPanel(const LeftPanel&) = delete;
    LeftPanel& operator=(const LeftPanel&) = delete;

    void render(float deltaTime);
    float getWidth() const { return panelWidth; }
    bool getShowAABBs() const { return showAABBs; }

private:
    float frameTime;
    float fps;

    float accumulatedTime;
    int frameCount;
    float displayedFrameTime;
    float displayedFPS;

    const float panelWidth = 200.0f;
    const float panelHeight = 600.0f;

    bool autoScroll = true;
    bool showAABBs = false;
};