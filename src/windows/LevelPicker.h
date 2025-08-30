#pragma once
#include <vector>
#include <string>

class LevelPicker
{
public:
    LevelPicker() = delete;

    static bool update(bool& showWindow, const std::vector<std::string>& levelNames, int& selectedLevelIndex);
};

