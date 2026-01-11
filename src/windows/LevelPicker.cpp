#include "LevelPicker.h"

#include <SDL3/SDL_render.h>
#include "imgui.h"

#include "parsers/SEF_Parser.h"
#include "utils/TracyProfiler.h"
#include "utils/TextureLoader.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"

LevelPicker::LevelPicker() :
    m_title("Level picker")
{

}

LevelPickerResult LevelPicker::update(bool& showWindow,
                                      SDL_Renderer* renderer,
                                      std::string_view rootDirectory,
                                      const std::vector<std::string>& singleLevelNames,
                                      const std::vector<std::string>& multiLevelNames,
                                      const StringHashTable<std::string>& levelHumanNamesDict,
                                      int& selectedLevelIndex)
{
    const ImGuiIO& io = ImGui::GetIO();
    LevelPickerResult result;

    const std::vector<std::string>& levelNames = (m_type == LevelType::kSingle) ? singleLevelNames
                                                                                : multiLevelNames;

    if ( showWindow && !ImGui::IsPopupOpen(m_title.data()) ) {
        ImGui::OpenPopup(m_title.data());

        if (!m_preview.isValid()) {
            //loadPreview(rootDirectory, levelNames[selectedLevelIndex], renderer);
        }
    }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(m_title.data(), &showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (!multiLevelNames.empty()) {
            if (ImGui::RadioButton("Single", m_type == LevelType::kSingle)) {
                m_type = LevelType::kSingle;
                selectedLevelIndex = 0;
                //loadPreview(rootDirectory, levelNames[selectedLevelIndex], renderer);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Multiplayer", m_type == LevelType::kMultiplayer)) {
                m_type = LevelType::kMultiplayer;
                selectedLevelIndex = 0;
                //loadPreview(rootDirectory, levelNames[selectedLevelIndex], renderer);
            }
        } else {
            m_type = LevelType::kSingle;
        }     

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
                    //loadPreview(rootDirectory, levelNames[selectedLevelIndex], renderer);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // if (m_preview.isValid()) {
        //     ImGui::Image((ImTextureID)m_preview.get(), ImVec2(m_preview->w, m_preview->h));
        // }

        if (ImGui::Button("Load")) {
            showWindow = false;

            result.selected = true;
            result.loadedLevelType = m_type;
            result.loadedLevelName = currentLevelName;
        }

        ImGui::EndPopup();
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

void LevelPicker::loadPreview(std::string_view rootDirectory,
                              std::string_view selectedLevelName,
                              SDL_Renderer* renderer)
{
    Tracy_ZoneScoped;
    char pathBuffer[768];
    StringUtils::formatToBuffer(pathBuffer, "{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelTypeToString(m_type), selectedLevelName);

    char pack[32];
    SEF_Parser::fastPackParse(pathBuffer, pack, nullptr);

    StringUtils::formatToBuffer(pathBuffer, "{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, pack);

    auto previewBuffer = FileUtils::loadJpegPhotoshopThumbnail(pathBuffer);
    TextureLoader::loadTextureFromMemory(previewBuffer, renderer, m_preview);
}
