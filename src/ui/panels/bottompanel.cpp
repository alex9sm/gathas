#include "bottompanel.hpp"
#include "../../core/directionallight.hpp"
#include "../../core/pointlight.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

BottomPanel::BottomPanel(Scene* scene, const std::string& assetsPath, DirectionalLight* dirLight, PointLight* pointLight)
    : scene(scene), assetsPath(assetsPath), dirLight(dirLight), pointLight(pointLight) {
    scanAssetFolders();
}

BottomPanel::~BottomPanel() {
}

void BottomPanel::scanAssetFolders() {
    assetFolders.clear();

    try {
        if (!fs::exists(assetsPath) || !fs::is_directory(assetsPath)) {
            std::cerr << "Assets path does not exist or is not a directory: " << assetsPath << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(assetsPath)) {
            if (entry.is_directory()) {
                assetFolders.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning asset folders: " << e.what() << std::endl;
    }
}

void BottomPanel::render() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowBgAlpha(0.7f);

    ImGui::Begin("Asset Browser", nullptr, window_flags);

    if (ImGui::Button("Refresh")) {
        scanAssetFolders();
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear Scene")) {
        if (scene) {
            scene->clear();
        }
        if (dirLight) {
            dirLight->setEnabled(false);
        }
        if (pointLight) {
            pointLight->setEnabled(false);
        }
    }

    ImGui::Separator();

    float availableHeight = ImGui::GetContentRegionAvail().y;
    float halfHeight = (availableHeight - ImGui::GetTextLineHeightWithSpacing() * 2 - 20) / 2;

    ImGui::Text("Models");
    ImGui::BeginChild("ModelsList", ImVec2(0, halfHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (size_t i = 0; i < assetFolders.size(); ++i) {
        bool isSelected = (selectedModelIndex == static_cast<int>(i));
        const std::string& modelName = assetFolders[i];

        if (ImGui::Selectable(modelName.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            selectedModelIndex = static_cast<int>(i);

            if (ImGui::IsMouseDoubleClicked(0) && scene) {
                std::string folderPath = assetsPath + "/" + modelName;

                try {
                    scene->loadModel(folderPath, modelName);
                }
                catch (const std::exception& e) {
                    std::cerr << "Failed to load model: " << e.what() << std::endl;
                }
            }
        }
    }

    ImGui::EndChild();

    ImGui::Separator();

    ImGui::Text("Actors");
    ImGui::BeginChild("ActorsList", ImVec2(0, halfHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    bool dirLightSelected = (selectedActorIndex == 0);
    if (ImGui::Selectable("Directional Light", dirLightSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
        selectedActorIndex = 0;

        if (ImGui::IsMouseDoubleClicked(0) && dirLight) {
            dirLight->setEnabled(true);
        }
    }

    bool pointLightSelected = (selectedActorIndex == 1);
    if (ImGui::Selectable("Point Light", pointLightSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
        selectedActorIndex = 1;

        if (ImGui::IsMouseDoubleClicked(0) && pointLight) {
            pointLight->setEnabled(true);
        }
    }

    ImGui::EndChild();

    ImGui::End();

    ImGui::PopStyleColor(5);
}
