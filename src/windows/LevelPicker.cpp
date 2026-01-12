#include "LevelPicker.h"

#include <SDL3/SDL_render.h>
#include "imgui.h"

#include "parsers/SEF_Parser.h"
#include "utils/TracyProfiler.h"
#include "utils/TextureLoader.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"

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

    if ( showWindow && !ImGui::IsPopupOpen(m_title.data()) ) {
        ImGui::OpenPopup(m_title.data());

        if (multiLevelNames.empty()) {
            m_type = LevelType::kSingle;
        }
        loadPreview(rootDirectory, levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex], renderer);
    }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(m_title.data(), &showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (!multiLevelNames.empty()) {
            if (ImGui::RadioButton("Single", m_type == LevelType::kSingle)) {
                m_type = LevelType::kSingle;
                selectedLevelIndex = 0;
                loadPreview(rootDirectory, levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex], renderer);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Multiplayer", m_type == LevelType::kMultiplayer)) {
                m_type = LevelType::kMultiplayer;
                selectedLevelIndex = 0;
                loadPreview(rootDirectory, levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex], renderer);
            }
        }

        char currentLevelNameBuffer[256];
        std::string_view currentLevelName = levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex];
        writeLevelHumanNameToBuffer(levelHumanNamesDict, currentLevelName, currentLevelNameBuffer);

        if (ImGui::BeginCombo("Levels", currentLevelNameBuffer)) {
            char levelNameBuffer[256];
            for (int i = 0; i < static_cast<int>(levelNames(singleLevelNames, multiLevelNames).size()); ++i) {
                bool isSelected = (i == selectedLevelIndex);

                writeLevelHumanNameToBuffer(levelHumanNamesDict, levelNames(singleLevelNames, multiLevelNames)[i], levelNameBuffer);
                if (ImGui::Selectable(levelNameBuffer, isSelected)) {
                    selectedLevelIndex = i;
                    loadPreview(rootDirectory, levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex], renderer);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if ( ImGui::Button("Load", ImVec2(70.0f, 0.0f)) ) {
            showWindow = false;

            result.selected = true;
            result.loadedLevelType = m_type;
            result.loadedLevelName = currentLevelName;
        }

        if (m_preview.isValid()) {
            ImGui::Dummy(ImVec2(0.0f, 2.0f));

            ImVec2 imageSize(m_preview->w, m_preview->h);
            //ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - imageSize.x) * 0.5f);

            ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

            ImGui::Image((ImTextureID)m_preview.get(), imageSize);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
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
    m_preview = {};

    char pathBuffer[768];
    StringUtils::formatToBuffer(pathBuffer, "{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelTypeToString(m_type), selectedLevelName);

    char pack[32];
    bool packParseOk = SEF_Parser::fastPackParse(pathBuffer, pack, &m_previewError);
    if (packParseOk) {
        StringUtils::formatToBuffer(pathBuffer, "{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, pack);

        auto previewBuffer = FileUtils::loadJpegPhotoshopThumbnail(pathBuffer, &m_previewError);
        if (!previewBuffer.empty()) {
            if (!TextureLoader::loadTextureFromMemory(previewBuffer, renderer, m_preview, &m_previewError)) {
                LogFmt("TextureLoader::loadTextureFromMemory error: {}", m_previewError);
            }
        } else {
            LogFmt("FileUtils::loadJpegPhotoshopThumbnail error: {}", m_previewError);
        }
    } else {
        LogFmt("SEF_Parser::fastPackParse error: {}", m_previewError);
    }
}
