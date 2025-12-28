#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <span>

#include "Types.h"

struct LevelPickerResult {
    bool selected = false;
    LevelType loadedLevelType;
    std::string_view loadedLevelName;
};

class LevelPicker {
public:
    LevelPicker();

    LevelPickerResult update(bool& showWindow,
                             const std::vector<std::string>& singleLevelNames,
                             const std::vector<std::string>& multiLevelNames,
                             const StringHashTable<std::string>& levelHumanNamesDict,
                             int& selectedLevelIndex);

    void writeLevelHumanNameToBuffer(const StringHashTable<std::string>& levelHumanNamesDict,
                                     std::string_view levelName,
                                     std::span<char> outLevelNameBuffer);

private:
    std::string_view m_title;
    LevelType m_type = LevelType::kSingle;
};
