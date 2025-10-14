#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <span>

#include "Types.h"

struct LevelVersion {
    uint16_t major = 0;
    uint16_t minor = 0;
};

struct MapSize {
    uint32_t pixelWidth = 1;
    uint32_t pixelHeight = 1;
};

struct PixelPosition {
    int32_t x = 0;
    int32_t y = 0;
};

struct LVL_Description {
    std::string name;
    uint16_t param1 = 0;
    uint16_t param2 = 0;
    uint32_t number = 0;
    PixelPosition position;
};

struct MaskDescription {
    uint32_t number = 0;
    uint32_t x = 0;
    uint32_t y = 0;
};

struct Weather {
    uint16_t type = 0;
    uint16_t intensity = 0;
};

struct CellGroup {
    std::string name;
    std::vector<TilePosition> cells;
};

struct Door {
    std::string sefName;
    std::string openAction;
    std::string closeAction;
    std::string cellGroup;
    std::string param1;
    std::string staticName;
};

struct ExtraSound {
    std::string path;
    float chunkPositionX = 0;
    float chunkPositionY = 0;
    float param03 = 0;
    float param04 = 0;
    float param05 = 0;
    float param06 = 0;
    float param07 = 0;
    float param08 = 0;
    uint32_t param09 = 0;
    uint32_t param10 = 0;
    uint32_t param11 = 0;
    uint32_t param12 = 0;
};

struct EnvironmentSoundHeader {
    int32_t param1 = 0;
    float param2 = 0;
    float param3 = 0;
    float param4 = 0;
};

struct EnvironmentSounds {
    EnvironmentSoundHeader header{};
    std::string levelTheme;
    std::string dayAmbience;
    std::string nightAmbience;
    std::vector<ExtraSound> otherSounds;
};

struct MapTile {
    uint16_t relief = 0; // Всё что ниже 2000 - проходимо, всё что выше 30000 - не простреливается (Точные значения требуют проверки)
    uint16_t sound = 0;  // Какой звук издаёт поверхность при контакте
    uint16_t mask = 0;   // Карта расположения масок уровня (объекты которые рисуется над героем)
};

using MapChunk = std::array<MapTile, 4>;
struct MapTiles {
    uint32_t chunkWidth = 0; // (Big cells)
    uint32_t chunkHeight = 0;
    std::vector<MapChunk> chunks;
};

struct LVL_Data {
    LevelVersion version;
    MapSize mapSize;
    MapTiles mapTiles;
    std::vector<MaskDescription> maskDescriptions;
    std::vector<LVL_Description> staticDescriptions;
    std::vector<LVL_Description> animationDescriptions;
    std::vector<LVL_Description> triggerDescriptions;
    std::vector<CellGroup> cellGroups;
    EnvironmentSounds environmentSounds;
    Weather weather;
    std::vector<Door> doors;
    uint32_t levelFloors = 0;
};

class LVL_Parser {
public:
    LVL_Parser() = delete;

    static bool parse(std::string_view lvlPath, LVL_Data& data, std::string* error);

private:
    friend struct ParsersPrivate;
    static consteval auto makeParsers();

    static void parseVersion(std::span<const uint8_t> block, LVL_Data& data);
    static void parseMapSize(std::span<const uint8_t> block, LVL_Data& data);
    static void parseMapTiles(std::span<const uint8_t> block, LVL_Data& data);
    static void parseMaskDescriptions(std::span<const uint8_t> block, LVL_Data& data);
    static void parseStaticDescriptions(std::span<const uint8_t> block, LVL_Data& data);
    static void parseAnimationDescriptions(std::span<const uint8_t> block, LVL_Data& data);
    static void parseTriggerDescriptions(std::span<const uint8_t> block, LVL_Data& data);
    static void parseCellGroups(std::span<const uint8_t> block, LVL_Data& data);
    static void parseSounds(std::span<const uint8_t> block, LVL_Data& data);
    static void parseWeather(std::span<const uint8_t> block, LVL_Data& data);
    static void parseDoors(std::span<const uint8_t> block, LVL_Data& data);
    static void parseLevelFloors(std::span<const uint8_t> block, LVL_Data& data);

    static void parseStructuredBlock(std::span<const uint8_t> block, std::vector<LVL_Description>& data);
};
