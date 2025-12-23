#include "scenepanel.hpp"
#include "../../core/directionallight.hpp"

ScenePanel::ScenePanel(DirectionalLight* light)
    : light(light) {
}

ScenePanel::~ScenePanel() {
}

void ScenePanel::render() {
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
    ImGui::SetNextWindowSize(ImVec2(280, 150), ImGuiCond_FirstUseEver);

    ImGui::Begin("Scene Settings", &isOpen, window_flags);

    ImGui::Text("Scene Settings");
    ImGui::Separator();

    ImGui::Text("Ambient Color");
    ImGui::ColorEdit3("##AmbientColor", light->getAmbientColorPtr(), ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Ambient Intensity");
    ImGui::SliderFloat("##AmbientIntensity", light->getAmbientIntensityPtr(), 0.0f, 1.0f);

    ImGui::End();

    ImGui::PopStyleColor(8);
}
