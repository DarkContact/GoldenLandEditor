#pragma once
#include <string_view>

#include "SDL3/SDL_render.h"
#include "parsers/SEF_Parser.h"

struct LevelData {
    std::string name;
    SDL_Texture* background = nullptr;
    SEF_Data sefData;
};

class Level
{
public:
    bool loadLevel(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType = "single");

    LevelData data() const;

private:
    std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const;
    std::string levelBackground(std::string_view rootDirectory, std::string_view levelPack) const;

    LevelData m_data;
};
