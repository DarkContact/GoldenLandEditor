#pragma once
#include <string_view>
#include <optional>

#include "imgui.h"

#include "parsers/SEF_Parser.h"
#include "parsers/SDB_Parser.h"
#include "parsers/LVL_Parser.h"
#include "parsers/LAO_Parser.h"
#include "Texture.h"
#include "Types.h"

enum class MapTilesMode {
    Relief,
    Sound,
    Mask
};

enum class CellGroupMode {
    Both,
    OnlySef,
    OnlyLvl
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

    bool showMapTiles = false;
    MapTilesMode mapTilesMode = MapTilesMode::Relief;

    bool showPersons = true;
    bool showEntrancePoints = true;

    bool showCellGroups = false;
    CellGroupMode cellGroupMode = CellGroupMode::Both;
    std::optional<int> highlightCellGroudIndex;

    bool showAnimations = true;
    bool showSounds = false;
};

struct LevelAnimation : public BaseAnimation {
    LevelAnimation(LVL_Description& description) :
        description(description) {}

    LVL_Description& description;  // Из lvlData
};

struct LevelData {
    std::string name;
    LevelType type;

    Texture background;
    Texture minimap;
    SEF_Data sefData;
    LVL_Data lvlData;
    SDB_Data sdbData;
    std::optional<LAO_Data> laoData;
    std::vector<LevelAnimation> animations;
    LevelImgui imgui;
};

class Level
{
public:
    ~Level() = default;

    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;

    Level(Level&& other) noexcept = default;
    Level& operator=(Level&& other) noexcept = default;

    static std::optional<Level> loadLevel(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, LevelType levelType, std::string* error);

    static std::string levelWindowName(std::string_view level, LevelType levelType);

    const LevelData& data() const;
    LevelData& data();

    std::string levelDir(std::string_view levelType) const;
    std::string levelPackDir() const;

    static const int tileWidth = 12;
    static const int tileHeight = 9;

    static const int chunkWidth = tileWidth * 2;
    static const int chunkHeight = tileHeight * 2;

private:
    Level() noexcept = default;

    static std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType);
    static std::string levelSdb(std::string_view rootDirectory, std::string_view level, std::string_view levelType);
    static std::string levelLvl(std::string_view rootDirectory, std::string_view levelPack);
    static std::string levelBackground(std::string_view rootDirectory, std::string_view levelPack);
    static std::string levelMinimap(std::string_view rootDirectory, std::string_view levelPack);
    static std::string levelLao(std::string_view rootDirectory, std::string_view levelPack);
    static std::string levelAnimationDir(std::string_view rootDirectory, std::string_view levelPack);
    static std::string levelAnimation(std::string_view rootDirectory, std::string_view levelPack, int index);

    LevelData m_data;
    std::string_view m_rootDirectory;
};
