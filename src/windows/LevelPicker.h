#pragma once
#include <vector>
#include <string>

#include "Level.h"
#include "SDL3/SDL_render.h"

class LevelPicker
{
public:
    LevelPicker() = delete;

    static void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& levelNames, Level& level);
};

