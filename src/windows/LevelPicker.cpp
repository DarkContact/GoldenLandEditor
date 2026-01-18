#include "LevelPicker.h"

#include <cmath>

#include <SDL3/SDL_render.h>
#include "imgui.h"
#include "imgui_internal.h"

#include "embedded_resources.h"
#include "graphics/TextureLoader.h"
#include "parsers/SEF_Parser.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"

LevelPicker::LevelPicker() :
    m_title("Load Level")
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
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));

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
            ImGui::Dummy({0, 0}); // Отступ
        }

        char currentLevelNameBuffer[256];
        std::string_view currentLevelName = levelNames(singleLevelNames, multiLevelNames)[selectedLevelIndex];
        writeLevelHumanNameToBuffer(levelHumanNamesDict, currentLevelName, currentLevelNameBuffer);

        if (ImGui::BeginCombo("##Levels", currentLevelNameBuffer, ImGuiComboFlags_HeightLarge)) {
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

        ImGui::Dummy({0, 0}); // Отступ
        ImVec2 bgPos = ImGui::GetCursorScreenPos();
        ImVec2 bgSize(128, 112);
        ImGui::Dummy(bgSize);

        if (m_preview.isValid()) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            ImVec2 imageSize(m_preview->w, m_preview->h);
            if (imageSize.x > bgSize.x) {
                float ratio = bgSize.x / imageSize.x;
                imageSize.x = bgSize.x;
                imageSize.y *= ratio;
                imageSize.y = std::ceil(imageSize.y);
            }

            bgPos.x += ImGui::GetContentRegionAvail().x * 0.5f - bgSize.x * 0.5f;
            ImVec2 imagePos(
                std::floor(bgPos.x + (bgSize.x - imageSize.x) * 0.5f),
                std::floor(bgPos.y + (bgSize.y - imageSize.y) * 0.5f)
            );

            ImRect imageBox(imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));

            drawList->AddImage((ImTextureID)m_preview.get(), imageBox.Min, imageBox.Max);
            imageBox.Expand(1);
            drawList->AddRect(imageBox.Min, imageBox.Max, IM_COL32(200, 200, 200, 255));
        }

        ImGui::Dummy({0, 0}); // Отступ
        if ( ImGui::Button("Load", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)) ) {
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
    bool loadPreviewOk = false;
    m_preview = {};

    char pathBuffer[768];
    StringUtils::formatToBuffer(pathBuffer, "{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelTypeToString(m_type), selectedLevelName);

    char pack[32];
    bool packParseOk = SEF_Parser::fastPackParse(pathBuffer, pack, &m_previewError);
    if (packParseOk) {
        StringUtils::formatToBuffer(pathBuffer, "{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, pack);

        auto previewBuffer = FileUtils::loadJpegPhotoshopThumbnail(pathBuffer, &m_previewError);
        if (!previewBuffer.empty()) {
            if (TextureLoader::loadTextureFromMemory(previewBuffer, renderer, m_preview, &m_previewError)) {
                loadPreviewOk = true;
            } else {
                LogFmt("loadTextureFromMemory error: {}", m_previewError);
            }
        } else {
            LogFmt("loadJpegPhotoshopThumbnail error: {}", m_previewError);
        }
    } else {
        LogFmt("fastPackParse error: {}", m_previewError);
    }

    if (!loadPreviewOk) {
        if (!TextureLoader::loadTextureFromMemory({question_bmp, question_bmp_size}, renderer, m_preview, &m_previewError)) {
            LogFmt("loadTextureFromMemory error: {}", m_previewError);
        }
    }
}
