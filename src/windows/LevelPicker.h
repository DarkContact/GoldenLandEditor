#pragma once
#include <vector>
#include <string>

#include "Level.h"

class LevelPicker
{
public:
    LevelPicker() = delete;

    static void update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& levelNames, Level& level);
};

