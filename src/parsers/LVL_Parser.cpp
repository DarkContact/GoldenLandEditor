#include "LVL_Parser.h"

#include "FileLoader.h"
#include "StringUtils.h"

LVL_Parser::LVL_Parser(std::string_view lvlPath) :
    m_filePath(lvlPath)
{

}

LVL_Data& LVL_Parser::parse() {
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
    LVL_Data d = m_data; // defaults
    for (auto& [name, block]: m_blocks) {
        if      (name == "BLK_LVER") d.version = parseVersion(block);
        else if (name == "BLK_MPSZ") d.mapSize = parseMapSize(block);
        else if (name == "BLK_WTHR") d.weather = parseWeather(block);
        else if (name == "BLK_LFLS") d.levelFloors = parseLevelFloors(block);
        else if (name == "BLK_CGRP") d.cellGroups = parseCellGroups(block);
        else if (name == "BLK_DOOR") d.doors = parseDoors(block);
        else if (name == "BLK_SENV") d.environmentSounds = parseSENV(block);
        else if (name == "BLK_SDSC") d.staticDescriptions = parseStructuredBlock(block);
        else if (name == "BLK_ADSC") d.animationDescriptions = parseStructuredBlock(block);
        else if (name == "BLK_TDSC") d.triggerDescription = parseStructuredBlock(block);
        else if (name == "BLK_MDSC") d.maskDescriptions = parseMaskDescriptions(block);
        else if (name == "BLK_MHDR") d.maskHDR = parseMaskHDR(block);
        // else ignore or keep raw
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

Weather LVL_Parser::parseWeather(const std::vector<uint8_t>& block) {
    if (block.size() < 4) return {};
    return {
        *reinterpret_cast<const uint16_t*>(&block[0]),
        *reinterpret_cast<const uint16_t*>(&block[2])
    };
}

uint32_t LVL_Parser::parseLevelFloors(const std::vector<uint8_t>& block) {
    if (block.size() < 4) return 0;
    return *reinterpret_cast<const uint32_t*>(&block[0]);
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

MaskHDR LVL_Parser::parseMaskHDR(const std::vector<uint8_t>& block) {
    MaskHDR hdr;
    if (block.size() < 8) return hdr;
    hdr.width  = *reinterpret_cast<const uint32_t*>(&block[0]);
    hdr.height = *reinterpret_cast<const uint32_t*>(&block[4]);

    const size_t tileSize = 6;
    const size_t chunkSize = 4;
    const size_t offsetStart = 8;

    size_t numChunks = (block.size() - offsetStart) / (tileSize * chunkSize);
    size_t offset = offsetStart;

    for (size_t i = 0; i < numChunks; ++i) {
        MHDRChunk chunk;
        for (size_t j = 0; j < chunkSize; ++j) {
            if (offset + tileSize > block.size()) break;
            MHDRTile tile;
            tile.maskNumber = *reinterpret_cast<const uint16_t*>(&block[offset]);
            tile.soundType  = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
            tile.tileType   = *reinterpret_cast<const uint16_t*>(&block[offset + 4]);
            chunk.push_back(tile);
            offset += tileSize;
        }
        hdr.chunks.push_back(chunk);
    }

    return hdr;
}

std::vector<LVLDescription> LVL_Parser::parseStructuredBlock(const std::vector<uint8_t>& block) {
    std::vector<LVLDescription> result;
    if (block.size() < 4) return result;
    size_t offset = 0;
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    for (uint32_t i = 0; i < count; ++i) {
        if (offset + 16 > block.size()) break;
        LVLDescription desc;
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

EnvironmentSounds LVL_Parser::parseSENV(const std::vector<uint8_t>& block) {
    EnvironmentSounds result;
    if (block.size() < 16) return result;
    size_t offset = 0;
    result.header.param1 = *reinterpret_cast<const int32_t*>(&block[offset]);
    offset += 4;
    result.header.param2 = static_cast<float>(*reinterpret_cast<const int32_t*>(&block[offset])) / 100.0f; offset += 4;
    result.header.param3 = static_cast<float>(*reinterpret_cast<const int32_t*>(&block[offset])) / 100.0f; offset += 4;
    result.header.param4 = static_cast<float>(*reinterpret_cast<const int32_t*>(&block[offset])) / 100.0f; offset += 4;

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
            reinterpret_cast<uint32_t*>(&s.param1)[p] = *reinterpret_cast<const uint32_t*>(&block[offset + p * 4]);
        offset += 32;
        s.param9  = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param10 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param11 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        s.param12 = *reinterpret_cast<const uint32_t*>(&block[offset]); offset += 4;
        result.otherSounds.push_back(s);
    }

    return result;
}
