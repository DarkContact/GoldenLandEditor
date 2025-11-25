#pragma once
#include <vector>
#include <string>

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
                             int& selectedLevelIndex);

private:
    bool m_onceWhenOpen = false;
    LevelType m_type = LevelType::kSingle;
};
