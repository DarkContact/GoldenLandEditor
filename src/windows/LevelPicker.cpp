#include "LevelPicker.h"

#include "imgui.h"
#include "utils/ImGuiWidgets.h"

LevelPickerResult LevelPicker::update(bool& showWindow, const std::vector<std::string>& singleLevelNames, const std::vector<std::string>& multiLevelNames, int& selectedLevelIndex)
{
    static bool onceWhenOpen = false;
    static LevelType type = LevelType::kSingle;
    LevelPickerResult result;

    if (showWindow && !onceWhenOpen) {
        ImGui::OpenPopup("Level picker");
        onceWhenOpen = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Level picker", &showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (!multiLevelNames.empty()) {
            if (ImGui::RadioButton("Single", type == LevelType::kSingle)) {
                type = LevelType::kSingle;
                selectedLevelIndex = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Multiplayer", type == LevelType::kMultiplayer)) {
                type = LevelType::kMultiplayer;
                selectedLevelIndex = 0;
            }
        } else {
            type = LevelType::kSingle;
        }

        ImGuiWidgets::ComboBoxWithIndex("Levels",
                                        (type == LevelType::kSingle) ? singleLevelNames : multiLevelNames,
                                        selectedLevelIndex);

        if (ImGui::Button("Load")) {
            showWindow = false;

            result.selected = true;
            result.loadedLevelType = type;
            result.loadedLevelName = (type == LevelType::kSingle) ? singleLevelNames[selectedLevelIndex]
                                                                    : multiLevelNames[selectedLevelIndex];
        }

        ImGui::EndPopup();
    }

    if (!showWindow) {
        onceWhenOpen = false;
    }

    return result;
}
