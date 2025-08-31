#pragma once
#include <string_view>

#include "SDL3/SDL_render.h"
#include "parsers/SEF_Parser.h"

struct LevelImgui {
    bool showMinimap = true;
};

struct LevelData {
    std::string name;
    SDL_Texture* background = nullptr;
    SDL_Texture* minimap = nullptr;
    SEF_Data sefData;
    LevelImgui imgui;
};

class Level
{
public:
    Level(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType = "single");
    ~Level();

    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;

    Level(Level&& other) noexcept;
    Level& operator=(Level&& other) noexcept;

    LevelData data() const;
    LevelData& data();

private:
    std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const;
    std::string levelBackground(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelMinimap(std::string_view rootDirectory, std::string_view levelPack) const;

    LevelData m_data;
};
