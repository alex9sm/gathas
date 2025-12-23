#include "pointlightpanel.hpp"
#include "../../core/pointlight.hpp"

PointLightPanel::PointLightPanel(PointLight* light)
    : light(light) {
}

PointLightPanel::~PointLightPanel() {
}

void PointLightPanel::render() {
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
    ImGui::SetNextWindowSize(ImVec2(280, 250), ImGuiCond_FirstUseEver);

    ImGui::Begin("Point Light", &isOpen, window_flags);

    ImGui::Text("Point Light");
    ImGui::Separator();

    ImGui::Text("Position");
    ImGui::SliderFloat("X##Position", light->getPositionPtr(), -1000.0f, 1000.0f);
    ImGui::SliderFloat("Y##Position", light->getPositionPtr() + 1, -1000.0f, 1000.0f);
    ImGui::SliderFloat("Z##Position", light->getPositionPtr() + 2, -1000.0f, 1000.0f);

    ImGui::Separator();

    ImGui::Text("Color");
    ImGui::ColorEdit3("##PointLightColor", light->getColorPtr(), ImGuiColorEditFlags_NoLabel);

    ImGui::Separator();

    ImGui::Text("Intensity");
    ImGui::SliderFloat("##PointIntensity", light->getIntensityPtr(), 0.0f, 100000.0f);

    ImGui::End();

    ImGui::PopStyleColor(8);
}
