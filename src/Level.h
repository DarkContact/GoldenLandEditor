#pragma once
#include <string_view>

#include "imgui.h"
#include "SDL3/SDL_render.h"
#include "parsers/SEF_Parser.h"
#include "parsers/LVL_Parser.h"

enum class MapDataMode {
    Relief,
    Sound,
    Mask
};

enum MapDataSound : uint16_t {
    Ground = 0,
    Grass = 1,
    Sand = 2,
    Wood = 3,
    Stone = 4,
    Water = 5,
    Snow = 6
};

struct LevelImgui {
    bool showMinimap = true;

    bool draggingMinimap = false;
    ImVec2 dragOffset = ImVec2(0, 0);

    bool minimapAnimating = false;
    ImVec2 minimapScrollStart = ImVec2(0, 0);
    ImVec2 minimapScrollTarget = ImVec2(0, 0);
    float minimapAnimTime = 0.0f;

    bool minimapHovered = false;

    bool draggingLevel = false;
    ImVec2 dragStartPos;
    ImVec2 scrollStart;

    bool showMetaInfo = false;

    bool showMapData = false;
    MapDataMode mapDataMode = MapDataMode::Relief;

    bool showPersons = true;
    bool showEntrancePoints = true;

    bool showCellGroups = false;
    std::optional<int> highlightCellGroudIndex;
};

struct LevelData {
    std::string name;
    SDL_Texture* background = nullptr;
    SDL_Texture* minimap = nullptr;
    SEF_Data sefData;
    LVL_Data lvlData;
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

    const LevelData& data() const;
    LevelData& data();

    static const int tileWidth = 12;
    static const int tileHeight = 9;

private:
    std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const;
    std::string levelLvl(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelBackground(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelMinimap(std::string_view rootDirectory, std::string_view levelPack) const;

    LevelData m_data;
};
