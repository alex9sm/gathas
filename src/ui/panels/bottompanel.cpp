#include "bottompanel.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

BottomPanel::BottomPanel(Scene* scene, const std::string& assetsPath)
    : scene(scene), assetsPath(assetsPath) {
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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));      // panel background
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));          // title bar
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));    // active title bar
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));           // border color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));             // text color

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
    }

    ImGui::Separator();

    ImGui::Text("Models: %zu", scene ? scene->getModelCount() : 0);

    ImGui::Separator();

    ImGui::BeginChild("AssetList", ImVec2(0, 0), true);

    for (size_t i = 0; i < assetFolders.size(); ++i) {
        bool isSelected = (selectedIndex == static_cast<int>(i));
        const std::string& modelName = assetFolders[i];

        if (ImGui::Selectable(modelName.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            selectedIndex = static_cast<int>(i);

            // handle double click to load
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

    ImGui::End();

    ImGui::PopStyleColor(5);
}
