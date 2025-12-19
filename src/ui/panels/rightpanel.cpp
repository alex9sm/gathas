#include "rightpanel.hpp"
#include "directionallightpanel.hpp"
#include "../../core/scene.hpp"
#include "../../core/directionallight.hpp"

RightPanel::RightPanel(Scene* scene, DirectionalLight* light, DirectionalLightPanel* dirLightPanel)
    : scene(scene), light(light), dirLightPanel(dirLightPanel), selectedObject("") {
}

RightPanel::~RightPanel() {
}

void RightPanel::render() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::SetNextWindowSize(ImVec2(280, 300), ImGuiCond_FirstUseEver);

    ImGui::Begin("Scene", nullptr, window_flags);

    ImGui::Text("Scene Objects");
    ImGui::Separator();

    // Camera object
    if (ImGui::Selectable("Camera", selectedObject == "Camera")) {
        selectedObject = "Camera";
    }

    // Directional Light object
    if (ImGui::Selectable("Directional Light", selectedObject == "Directional Light")) {
        selectedObject = "Directional Light";
        dirLightPanel->setOpen(true);
    }

    // Models from scene
    if (scene) {
        size_t modelCount = scene->getModelCount();
        for (size_t i = 0; i < modelCount; ++i) {
            const Scene::Model* model = scene->getModel(i);
            if (model) {
                std::string label = model->name;
                if (ImGui::Selectable(label.c_str(), selectedObject == label)) {
                    selectedObject = label;
                }
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(9);
}
