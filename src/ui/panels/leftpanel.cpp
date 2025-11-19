#include "leftpanel.hpp"
#include <glm/common.hpp>

LeftPanel::LeftPanel()
    : frameTime(0.0f), fps(0.0f),
    accumulatedTime(0.0f), frameCount(0),
    displayedFrameTime(0.0f), displayedFPS(0.0f),
    panelWidth(300.0f),
    isDraggingSplitter(false) {
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

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->Size.y), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin("Debug", nullptr, window_flags);
    ImGui::Text("Frame Time: %.2f ms", displayedFrameTime);
    ImGui::Text("FPS: %.1f", displayedFPS);
    ImGui::End();

    ImGui::PopStyleColor(5);

    renderSplitter();
}

void LeftPanel::renderSplitter() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 splitterMin(viewport->Pos.x + panelWidth, viewport->Pos.y);
    ImVec2 splitterMax(viewport->Pos.x + panelWidth + splitterWidth, viewport->Pos.y + viewport->Size.y);

    ImGui::SetNextWindowPos(splitterMin);
    ImGui::SetNextWindowSize(ImVec2(splitterWidth, viewport->Size.y));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags splitter_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("##Splitter", nullptr, splitter_flags);

    bool isHovered = ImGui::IsWindowHovered();

    if (isHovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(splitterMin, splitterMax, IM_COL32(100, 100, 100, 180));
    }

    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        isDraggingSplitter = true;
    }

    if (isDraggingSplitter) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            float mouseX = ImGui::GetMousePos().x;
            float newWidth = mouseX - viewport->Pos.x;

            panelWidth = glm::clamp(newWidth, minWidth, maxWidth);
        }
        else {
            isDraggingSplitter = false;
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}