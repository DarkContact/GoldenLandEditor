#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <map>

struct MapSize {
    uint32_t pixelWidth = 1;
    uint32_t pixelHeight = 1;
};

struct PixelPosition {
    uint32_t x = 0;
    uint32_t y = 0;
};

struct LVL_Description {
    std::string name;
    uint16_t param1 = 0;
    uint16_t param2 = 0;
    uint32_t number = 0;
    PixelPosition position{};
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
    uint16_t x = 0;
    uint16_t y = 0;
};
using CellGroups = std::map<std::string, std::vector<CellGroup>>;

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
    float param1 = 0;
    float param2 = 0;
    float param3 = 0;
    float param4 = 0;
    float param5 = 0;
    float param6 = 0;
    float param7 = 0;
    float param8 = 0;
    uint32_t param9 = 0;
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
    std::string version;
    MapSize mapSize;
    MapTiles mapTiles;
    std::vector<MaskDescription> maskDescriptions;
    std::vector<LVL_Description> staticDescriptions;
    std::vector<LVL_Description> animationDescriptions;
    std::vector<LVL_Description> triggerDescription;
    CellGroups cellGroups;
    EnvironmentSounds environmentSounds;
    Weather weather;
    std::vector<Door> doors;
    uint32_t levelFloors = 0;
};

class LVL_Parser
{
public:
    LVL_Parser(std::string_view lvlPath);

    LVL_Data& parse();

private:
    void extractBlocks(const std::vector<uint8_t>& dataBuf);

    LVL_Data interpretData();

    std::string parseVersion(const std::vector<uint8_t>& block);
    MapSize parseMapSize(const std::vector<uint8_t>& block);
    MapTiles parseMapTiles(const std::vector<uint8_t>& block);
    std::vector<MaskDescription> parseMaskDescriptions(const std::vector<uint8_t>& block);
    std::vector<LVL_Description> parseStructuredBlock(const std::vector<uint8_t>& block);
    CellGroups parseCellGroups(const std::vector<uint8_t>& block);
    EnvironmentSounds parseSoundEnv(const std::vector<uint8_t>& block);
    Weather parseWeather(const std::vector<uint8_t>& block);
    std::vector<Door> parseDoors(const std::vector<uint8_t>& block);
    uint32_t parseLevelFloors(const std::vector<uint8_t>& block);

    std::string m_filePath;
    std::map<std::string, std::vector<uint8_t>> m_blocks;
    LVL_Data m_data;
};
