#include "LVL_Parser.h"

#include "utils/FileLoader.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

LVL_Parser::LVL_Parser(std::string_view lvlPath) :
    m_filePath(lvlPath)
{

}

LVL_Data& LVL_Parser::parse() {
    Tracy_ZoneScopedN("LVL_Parser::parse");
    auto buffer = FileLoader::loadFile(m_filePath);
    extractBlocks(buffer);
    m_data = interpretData();
    return m_data;
}

void LVL_Parser::extractBlocks(const std::vector<uint8_t>& dataBuf) {
    size_t index = 0, n = dataBuf.size();
    while (index + 12 <= n) {
        std::string blockId(reinterpret_cast<const char*>(&dataBuf[index]), 8);
        uint32_t blockSize = *reinterpret_cast<const uint32_t*>(&dataBuf[index + 8]);
        index += 12;
        if (index + blockSize > n) break;
        m_blocks[blockId] = std::vector<uint8_t>(dataBuf.begin()+index, dataBuf.begin()+index+blockSize);
        index += blockSize;
    }
}

LVL_Data LVL_Parser::interpretData() {
    LVL_Data d = m_data;
    for (auto& [name, block]: m_blocks) {
        if      (name == "BLK_LVER") d.version = parseVersion(block);
        else if (name == "BLK_MPSZ") d.mapSize = parseMapSize(block);
        else if (name == "BLK_MHDR") d.mapTiles = parseMapTiles(block);
        else if (name == "BLK_MDSC") d.maskDescriptions = parseMaskDescriptions(block);
        else if (name == "BLK_SDSC") d.staticDescriptions = parseStructuredBlock(block);
        else if (name == "BLK_ADSC") d.animationDescriptions = parseStructuredBlock(block);
        else if (name == "BLK_TDSC") d.triggerDescription = parseStructuredBlock(block);
        else if (name == "BLK_CGRP") d.cellGroups = parseCellGroups(block);
        else if (name == "BLK_SENV") d.environmentSounds = parseSoundEnv(block);
        else if (name == "BLK_WTHR") d.weather = parseWeather(block);
        else if (name == "BLK_DOOR") d.doors = parseDoors(block);
        else if (name == "BLK_LFLS") d.levelFloors = parseLevelFloors(block);
    }
    return d;
}

std::string LVL_Parser::parseVersion(const std::vector<uint8_t>& block) {
    if (block.size() < 4) return "-1";
    uint16_t minor = *reinterpret_cast<const uint16_t*>(&block[0]);
    uint16_t major = *reinterpret_cast<const uint16_t*>(&block[2]);
    return std::to_string(major) + "." + std::to_string(minor);
}

MapSize LVL_Parser::parseMapSize(const std::vector<uint8_t>& block) {
    if (block.size() < 8) return {};
    return {
        *reinterpret_cast<const uint32_t*>(&block[0]),
        *reinterpret_cast<const uint32_t*>(&block[4])
    };
}

MapTiles LVL_Parser::parseMapTiles(const std::vector<uint8_t>& block) {
    MapTiles mapTiles;
    if (block.size() < 8) return mapTiles;
    mapTiles.chunkWidth  = *reinterpret_cast<const uint32_t*>(&block[0]);
    mapTiles.chunkHeight = *reinterpret_cast<const uint32_t*>(&block[4]);

    size_t offset = 8;
    const size_t numChunks = mapTiles.chunkWidth * mapTiles.chunkHeight;
    mapTiles.chunks.reserve(numChunks);
    for (size_t i = 0; i < numChunks; ++i) {
        MapChunk chunk;
        for (size_t j = 0; j < chunk.size(); ++j) {
            MapTile tile;
            tile.relief = *reinterpret_cast<const uint16_t*>(&block[offset]);
            tile.sound  = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
            tile.mask   = *reinterpret_cast<const uint16_t*>(&block[offset + 4]);
            chunk[j] = tile;
            offset += 6;
        }
        mapTiles.chunks.push_back(chunk);
    }

    return mapTiles;
}

std::vector<MaskDescription> LVL_Parser::parseMaskDescriptions(const std::vector<uint8_t>& block) {
    std::vector<MaskDescription> result;
    if (block.size() < 4) return result;
    size_t offset = 0;
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    for (uint32_t i = 0; i < count && offset + 16 <= block.size(); ++i) {
        offset += 4; // skip padding
        MaskDescription m;
        m.number = *reinterpret_cast<const uint32_t*>(&block[offset]);
        m.x      = *reinterpret_cast<const uint32_t*>(&block[offset + 4]);
        m.y      = *reinterpret_cast<const uint32_t*>(&block[offset + 8]);
        offset += 12;
        result.push_back(m);
    }
    return result;
}

std::vector<LVL_Description> LVL_Parser::parseStructuredBlock(const std::vector<uint8_t>& block) {
    std::vector<LVL_Description> result;
    if (block.size() < 4) return result;
    size_t offset = 0;
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    for (uint32_t i = 0; i < count; ++i) {
        if (offset + 16 > block.size()) break;
        LVL_Description desc;
        desc.param1 = *reinterpret_cast<const uint16_t*>(&block[offset]);
        desc.param2 = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
        desc.number = *reinterpret_cast<const uint32_t*>(&block[offset + 4]);
        desc.position.x = *reinterpret_cast<const uint32_t*>(&block[offset + 8]);
        desc.position.y = *reinterpret_cast<const uint32_t*>(&block[offset + 12]);
        offset += 16;
        desc.name = StringUtils::readStringWithLength(block, offset);
        result.push_back(desc);
    }

    return result;
}

CellGroups LVL_Parser::parseCellGroups(const std::vector<uint8_t>& block) {
    CellGroups groups;
    if (block.size() < 4) return groups;
    size_t offset = 0;
    uint32_t groupCount = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    for (uint32_t i = 0; i < groupCount; ++i) {
        std::string groupName = StringUtils::readStringWithLength(block, offset);
        if (offset + 4 > block.size()) break;
        uint32_t groupSize = *reinterpret_cast<const uint32_t*>(&block[offset]);
        offset += 4;
        std::vector<CellGroup> cells;
        for (uint32_t j = 0; j < groupSize; ++j) {
            if (offset + 4 > block.size()) break;
            CellGroup cg;
            cg.x = *reinterpret_cast<const uint16_t*>(&block[offset]);
            cg.y = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
            offset += 4;
            cells.push_back(cg);
        }
        groups[groupName] = cells;
    }

    return groups;
}

EnvironmentSounds LVL_Parser::parseSoundEnv(const std::vector<uint8_t>& block) {
    EnvironmentSounds result;
    if (block.size() < 16) return result;

    size_t offset = 0;
    result.header.param1 = *reinterpret_cast<const int32_t*>(&block[offset]);
    result.header.param2 = *reinterpret_cast<const float*>(&block[offset + 4]);
    result.header.param3 = *reinterpret_cast<const float*>(&block[offset + 8]);
    result.header.param4 = *reinterpret_cast<const float*>(&block[offset + 12]);
    offset += 16;

    if (offset + 4 > block.size()) return result;
    uint32_t soundCount = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    result.levelTheme = StringUtils::readStringWithLength(block, offset);
    result.dayAmbience = StringUtils::readStringWithLength(block, offset);
    result.nightAmbience = StringUtils::readStringWithLength(block, offset);

    for (uint32_t i = 0; i < soundCount; ++i) {
        ExtraSound s;
        s.path = StringUtils::readStringWithLength(block, offset);
        if (offset + 48 > block.size()) break;
        for (int p = 0; p < 8; ++p)
            reinterpret_cast<float*>(&s.param1)[p] = *reinterpret_cast<const float*>(&block[offset + p * 4]);
        offset += 32;
        s.param9  = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param10 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param11 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param12 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        result.otherSounds.push_back(s);
    }

    return result;
}

Weather LVL_Parser::parseWeather(const std::vector<uint8_t>& block) {
    if (block.size() < 4) return {};
    return {
        *reinterpret_cast<const uint16_t*>(&block[0]),
        *reinterpret_cast<const uint16_t*>(&block[2])
    };
}

std::vector<Door> LVL_Parser::parseDoors(const std::vector<uint8_t>& block) {
    std::vector<Door> result;
    if (block.size() < 4) return result;
    size_t offset = 0;
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    for (uint32_t i = 0; i < count; ++i) {
        Door d;
        d.sefName = StringUtils::readStringWithLength(block, offset);
        d.openAction = StringUtils::readStringWithLength(block, offset);
        d.closeAction = StringUtils::readStringWithLength(block, offset);
        d.cellGroup = StringUtils::readStringWithLength(block, offset);
        d.param1 = StringUtils::readStringWithLength(block, offset);
        d.staticName = StringUtils::readStringWithLength(block, offset);
        result.push_back(d);
    }

    return result;
}

uint32_t LVL_Parser::parseLevelFloors(const std::vector<uint8_t>& block) {
    if (block.size() < 4) return 0;
    return *reinterpret_cast<const uint32_t*>(&block[0]);
}
