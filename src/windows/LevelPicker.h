#pragma once
#include <vector>
#include <string>

class LevelPicker
{
public:
    LevelPicker() = delete;

    static void update(bool& showWindow, const std::vector<std::string>& levelNames);
};

