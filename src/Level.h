#pragma once
#include <string_view>

#include "imgui.h"
#include "Texture.h"
#include "parsers/SEF_Parser.h"
#include "parsers/LVL_Parser.h"
#include "parsers/LAO_Parser.h"

enum class MapTilesMode {
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

    bool showMapTiles = false;
    MapTilesMode mapTilesMode = MapTilesMode::Relief;

    bool showPersons = true;
    bool showEntrancePoints = true;

    bool showCellGroups = false;
    std::optional<int> highlightCellGroudIndex;

    bool showAnimations = true;
};

struct Animation {
    Animation(LVL_Description& description) :
        description(description) {}

    const Texture& currentTexture() const {
        return textures[currentFrame];
    }

    void update(uint64_t timeMs) {
        if (lastUpdate == 0) {
            lastUpdate = timeMs;
            return;
        }

        if (timeMs - lastUpdate >= duration) {
            nextFrame();
            lastUpdate = timeMs;
        }
    }

    void stop() {
        currentFrame = 0;
        lastUpdate = 0;
    }

    std::vector<Texture> textures;
    LVL_Description& description;  // Из lvlData
    uint32_t duration = 0;         // Из laoData


private:
    void nextFrame() {
        ++currentFrame;
        if (currentFrame == textures.size()) {
            currentFrame = 0;
        }
    }

    uint32_t currentFrame = 0;
    uint64_t lastUpdate = 0;
};

struct LevelData {
    std::string name;
    Texture background;
    Texture minimap;
    SEF_Data sefData;
    LVL_Data lvlData;
    std::optional<LAO_Data> laoData;
    std::vector<Animation> animations;
    LevelImgui imgui;
};

class Level
{
public:
    Level(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType = "single");
    ~Level() = default;

    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;

    Level(Level&& other) noexcept = default;
    Level& operator=(Level&& other) noexcept = default;

    const LevelData& data() const;
    LevelData& data();

    static const int tileWidth = 12;
    static const int tileHeight = 9;

private:
    std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const;
    std::string levelLvl(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelBackground(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelMinimap(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelLao(std::string_view rootDirectory, std::string_view levelPack) const;
    std::string levelAnimation(std::string_view rootDirectory, std::string_view levelPack, int index) const;

    LevelData m_data;
};
