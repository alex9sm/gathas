#include "leftpanel.hpp"
#include <glm/common.hpp>

LeftPanel::LeftPanel()
    : frameTime(0.0f), fps(0.0f),
    accumulatedTime(0.0f), frameCount(0),
    displayedFrameTime(0.0f), displayedFPS(0.0f) {
}

LeftPanel::~LeftPanel() {
}

void LeftPanel::render(float deltaTime) {
    accumulatedTime += deltaTime;
    frameCount++;

    if (accumulatedTime >= 1.0f) {
        displayedFPS = static_cast<float>(frameCount) / accumulatedTime;
        displayedFrameTime = (accumulatedTime / static_cast<float>(frameCount)) * 1000.0f;

        accumulatedTime = 0.0f;
        frameCount = 0;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));      // panel background
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));          // title bar
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));    // active title bar
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));           // border color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));           //text color

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowBgAlpha(0.7f);

    ImGui::Begin("Debug", nullptr, window_flags);
    ImGui::Text("Frame Time: %.2f ms", displayedFrameTime);
    ImGui::Text("FPS: %.1f", displayedFPS);
    ImGui::End();

    ImGui::PopStyleColor(5);
}