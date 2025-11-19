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

private:
    float frameTime;
    float fps;

    float accumulatedTime;
    int frameCount;
    float displayedFrameTime;
    float displayedFPS;

    float panelWidth;
    const float minWidth = 200.0f;
    const float maxWidth = 600.0f;
    const float splitterWidth = 4.0f;

    bool isDraggingSplitter;

    void renderSplitter();
};