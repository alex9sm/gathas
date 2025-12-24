#include "camerapanel.hpp"
#include "../../core/camera.hpp"

CameraPanel::CameraPanel(Camera* camera)
    : camera(camera) {
}

CameraPanel::~CameraPanel() {
}

void CameraPanel::render() {
    if (!isOpen) return;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.4f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.5f, 0.5f, 0.9f, 1.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::SetNextWindowSize(ImVec2(280, 100), ImGuiCond_FirstUseEver);

    ImGui::Begin("Camera", &isOpen, window_flags);

    ImGui::Text("Camera Settings");
    ImGui::Separator();

    ImGui::Text("Movement Speed");
    ImGui::SliderFloat("##Speed", camera->getSpeedPtr(), 10.0f, 2000.0f);

    ImGui::Separator();
    ImGui::Text("Debug Frustum Culling");

    bool useDebugFov = camera->getUseDebugCullingFov();
    if (ImGui::Checkbox("Use Debug Culling FOV", &useDebugFov)) {
        camera->setUseDebugCullingFov(useDebugFov);
    }

    if (useDebugFov) {
        float debugFov = camera->getDebugCullingFov();
        if (ImGui::SliderFloat("Culling FOV", &debugFov, 10.0f, 120.0f)) {
            camera->setDebugCullingFov(debugFov);
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(8);
}
