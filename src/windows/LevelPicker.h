#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <span>

#include "Texture.h"
#include "Types.h"

struct SDL_Renderer;

struct LevelPickerResult {
    bool selected = false;
    LevelType loadedLevelType;
    std::string_view loadedLevelName;
};

class LevelPicker {
public:
    LevelPicker();

    LevelPickerResult update(bool& showWindow,
                             SDL_Renderer* renderer,
                             std::string_view rootDirectory,
                             const std::vector<std::string>& singleLevelNames,
                             const std::vector<std::string>& multiLevelNames,
                             const StringHashTable<std::string>& levelHumanNamesDict,
                             int& selectedLevelIndex);

    void writeLevelHumanNameToBuffer(const StringHashTable<std::string>& levelHumanNamesDict,
                                     std::string_view levelName,
                                     std::span<char> outLevelNameBuffer);

    void loadPreview(std::string_view rootDirectory,
                     std::string_view selectedLevelName,
                     SDL_Renderer* renderer);

private:
    std::string_view m_title;
    LevelType m_type = LevelType::kSingle;
    Texture m_preview;
};
