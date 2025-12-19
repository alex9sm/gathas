#include "directionallightpanel.hpp"
#include "../../core/directionallight.hpp"

DirectionalLightPanel::DirectionalLightPanel(DirectionalLight* light)
    : light(light) {
}

DirectionalLightPanel::~DirectionalLightPanel() {
}

void DirectionalLightPanel::render() {
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
    ImGui::SetNextWindowSize(ImVec2(280, 300), ImGuiCond_FirstUseEver);

    ImGui::Begin("Directional Light", &isOpen, window_flags);

    ImGui::Text("Directional Light");
    ImGui::Separator();

    ImGui::Text("Direction");
    ImGui::SliderFloat("X##Direction", light->getDirectionPtr(), -1.0f, 1.0f);
    ImGui::SliderFloat("Y##Direction", light->getDirectionPtr() + 1, -1.0f, 1.0f);
    ImGui::SliderFloat("Z##Direction", light->getDirectionPtr() + 2, -1.0f, 1.0f);

    ImGui::Separator();

    ImGui::Text("Light Color");
    ImGui::ColorEdit3("##LightColor", light->getColorPtr(), ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Intensity");
    ImGui::SliderFloat("##Intensity", light->getIntensityPtr(), 0.0f, 5.0f);

    ImGui::Separator();
    ImGui::Text("Ambient");

    ImGui::Text("Ambient Color");
    ImGui::ColorEdit3("##AmbientColor", light->getAmbientColorPtr(), ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Ambient Intensity");
    ImGui::SliderFloat("##AmbientIntensity", light->getAmbientIntensityPtr(), 0.0f, 1.0f);

    ImGui::Separator();
    ImGui::Text("Specular");

    ImGui::Text("Specular Power");
    ImGui::SliderFloat("##SpecularPower", light->getSpecularPowerPtr(), 1.0f, 128.0f);

    ImGui::End();

    ImGui::PopStyleColor(8);
}
