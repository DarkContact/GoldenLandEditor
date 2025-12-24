#pragma once
#include <vector>
#include <string>
#include <span>

#include "Types.h"

struct LevelPickerResult {
    bool selected = false;
    LevelType loadedLevelType;
    std::string_view loadedLevelName;
};

class LevelPicker {
public:
    LevelPickerResult update(bool& showWindow,
                             const std::vector<std::string>& singleLevelNames,
                             const std::vector<std::string>& multiLevelNames,
                             const StringHashTable<std::string>& levelHumanNamesDict,
                             int& selectedLevelIndex);

    void writeLevelHumanNameToBuffer(const StringHashTable<std::string>& levelHumanNamesDict,
                                     std::string_view levelName,
                                     std::span<char> outLevelNameBuffer);

private:
    bool m_onceWhenOpen = false;
    LevelType m_type = LevelType::kSingle;
};
