#include "LevelPicker.h"

#include "imgui.h"

#include "utils/StringUtils.h"

LevelPickerResult LevelPicker::update(bool& showWindow,
                                      const std::vector<std::string>& singleLevelNames,
                                      const std::vector<std::string>& multiLevelNames,
                                      const StringHashTable<std::string>& levelHumanNamesDict,
                                      int& selectedLevelIndex)
{
    const ImGuiIO& io = ImGui::GetIO();
    LevelPickerResult result;

    if (showWindow && !m_onceWhenOpen) {
        ImGui::OpenPopup("Level picker");
        m_onceWhenOpen = true;
    }

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

        const std::vector<std::string>& levelNames = (m_type == LevelType::kSingle) ? singleLevelNames
                                                                                    : multiLevelNames;

        char currentLevelNameBuffer[256];
        std::string_view currentLevelName = levelNames[selectedLevelIndex];
        writeLevelHumanNameToBuffer(levelHumanNamesDict, currentLevelName, currentLevelNameBuffer);

        if (ImGui::BeginCombo("Levels", currentLevelNameBuffer)) {
            char levelNameBuffer[256];
            for (int i = 0; i < static_cast<int>(levelNames.size()); ++i) {
                bool isSelected = (i == selectedLevelIndex);

                writeLevelHumanNameToBuffer(levelHumanNamesDict, levelNames[i], levelNameBuffer);
                if (ImGui::Selectable(levelNameBuffer, isSelected)) {
                    selectedLevelIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Load")) {
            showWindow = false;

            result.selected = true;
            result.loadedLevelType = m_type;
            result.loadedLevelName = currentLevelName;
        }

        ImGui::EndPopup();
    }

    if (!showWindow) {
        m_onceWhenOpen = false;
    }

    return result;
}

void LevelPicker::writeLevelHumanNameToBuffer(const StringHashTable<std::string>& levelHumanNamesDict,
                                              std::string_view levelName,
                                              std::span<char> outLevelNameBuffer)
{
    if (auto it = levelHumanNamesDict.find(levelName); it != levelHumanNamesDict.cend()) {
        std::string_view levelNameHuman = it->second;
        StringUtils::formatToBuffer(outLevelNameBuffer, "{} [{}]", levelName, levelNameHuman);
    } else {
        StringUtils::formatToBuffer(outLevelNameBuffer, "{}", levelName);
    }
}
