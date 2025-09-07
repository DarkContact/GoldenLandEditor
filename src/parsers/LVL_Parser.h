#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>

struct MapSize { uint32_t width = 1, height = 1; };
struct PixelPosition { uint32_t x = 0, y = 0; };
struct LVLDescription { std::string name; uint16_t param1 = 0, param2 = 0; uint32_t number = 0; PixelPosition position{}; };
struct MaskDescription { uint32_t number = 0, x = 0, y = 0; };
struct Weather { uint16_t type = 0, intensity = 0; };
struct CellGroup { uint16_t x = 0, y = 0; };
using CellGroups = std::map<std::string, std::vector<CellGroup>>;

struct Door {
    std::string sefName, openAction, closeAction, cellGroup, param1, staticName;
};

struct ExtraSound {
    std::string path;
    uint32_t param1=0, param2=0, param3=0, param4=0, param5=0, param6=0, param7=0, param8=0;
    uint32_t param9=0, param10=0, param11=0, param12=0;
};

struct EnvironmentSoundHeader {
    int32_t param1=0;
    int32_t param2=0, param3=0, param4=0;
};

struct EnvironmentSounds {
    EnvironmentSoundHeader header{};
    std::string levelTheme, dayAmbience, nightAmbience;
    std::vector<ExtraSound> otherSounds;
};

struct MHDRTile { uint16_t maskNumber=0, soundType=0, tileType=0; };
using MHDRChunk = std::vector<MHDRTile>; // 4 tiles per chunk
struct MaskHDR {
    uint32_t width = 1, height = 1;
    std::vector<MHDRChunk> chunks;
};

struct LVL_Data {
    std::string version = "-1";
    MapSize mapSize;
    Weather weather;
    uint32_t levelFloors = 0;
    std::vector<LVLDescription> staticDescriptions, animationDescriptions, triggerDescription;
    std::vector<MaskDescription> maskDescriptions;
    CellGroups cellGroups;
    std::vector<Door> doors;
    EnvironmentSounds environmentSounds;
    MaskHDR maskHDR;
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
    Weather parseWeather(const std::vector<uint8_t>& block);
    uint32_t parseLevelFloors(const std::vector<uint8_t>& block);
    std::vector<MaskDescription> parseMaskDescriptions(const std::vector<uint8_t>& block);
    MaskHDR parseMaskHDR(const std::vector<uint8_t>& block);
    std::vector<LVLDescription> parseStructuredBlock(const std::vector<uint8_t>& block);
    std::vector<Door> parseDoors(const std::vector<uint8_t>& block);
    CellGroups parseCellGroups(const std::vector<uint8_t>& block);
    EnvironmentSounds parseSENV(const std::vector<uint8_t>& block);

    std::string m_filePath;
    std::map<std::string, std::vector<uint8_t>> m_blocks;
    LVL_Data m_data;
};
