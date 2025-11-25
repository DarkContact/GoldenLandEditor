#include "LevelPicker.h"

#include "imgui.h"
#include "utils/ImGuiWidgets.h"

LevelPickerResult LevelPicker::update(bool& showWindow,
                                      const std::vector<std::string>& singleLevelNames,
                                      const std::vector<std::string>& multiLevelNames,
                                      int& selectedLevelIndex)
{
    LevelPickerResult result;

    if (showWindow && !m_onceWhenOpen) {
        ImGui::OpenPopup("Level picker");
        m_onceWhenOpen = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Level picker", &showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (!multiLevelNames.empty()) {
            if (ImGui::RadioButton("Single", m_type == LevelType::kSingle)) {
                m_type = LevelType::kSingle;
                selectedLevelIndex = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Multiplayer", m_type == LevelType::kMultiplayer)) {
                m_type = LevelType::kMultiplayer;
                selectedLevelIndex = 0;
            }
        } else {
            m_type = LevelType::kSingle;
        }

        ImGuiWidgets::ComboBoxWithIndex("Levels",
                                        (m_type == LevelType::kSingle) ? singleLevelNames : multiLevelNames,
                                        selectedLevelIndex);

        if (ImGui::Button("Load")) {
            showWindow = false;

            result.selected = true;
            result.loadedLevelType = m_type;
            result.loadedLevelName = (m_type == LevelType::kSingle) ? singleLevelNames[selectedLevelIndex]
                                                                    : multiLevelNames[selectedLevelIndex];
        }

        ImGui::EndPopup();
    }

    if (!showWindow) {
        m_onceWhenOpen = false;
    }

    return result;
}
