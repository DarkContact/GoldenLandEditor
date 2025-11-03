#include "LVL_Parser.h"

#include <cassert>
#include <cstring>

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

using namespace std::literals::string_view_literals;

using BlockParser = void(*)(std::span<const uint8_t>, LVL_Data&);
struct BlockParserEntry {
    std::string_view name;
    BlockParser parser;
};

enum Blocks {
    BLK_LVER, BLK_MPSZ, BLK_MHDR, BLK_MDSC,
    BLK_SDSC, BLK_ADSC, BLK_TDSC, BLK_CGRP,
    BLK_SENV, BLK_WTHR, BLK_DOOR, BLK_LFLS
};

consteval auto LVL_Parser::makeParsers() {
    return std::to_array<BlockParserEntry>({
        { "BLK_LVER"sv, &parseVersion },
        { "BLK_MPSZ"sv, &parseMapSize },
        { "BLK_MHDR"sv, &parseMapTiles },
        { "BLK_MDSC"sv, &parseMaskDescriptions },
        { "BLK_SDSC"sv, &parseStaticDescriptions },
        { "BLK_ADSC"sv, &parseAnimationDescriptions },
        { "BLK_TDSC"sv, &parseTriggerDescriptions },
        { "BLK_CGRP"sv, &parseCellGroups },
        { "BLK_SENV"sv, &parseSounds },
        { "BLK_WTHR"sv, &parseWeather },
        { "BLK_DOOR"sv, &parseDoors },
        { "BLK_LFLS"sv, &parseLevelFloors }
    });
}

struct ParsersPrivate {
    static constexpr auto kParsers = LVL_Parser::makeParsers();
};

bool LVL_Parser::parse(std::string_view lvlPath, LVL_Data& data, std::string* error) {
    Tracy_ZoneScoped;
    auto fileData = FileUtils::loadFile(lvlPath, error);
    if (fileData.empty()) {
        return false;
    }

    // В файле уровня должны присутствовать все 12 блоков данных
    // Блоки данных следуют друг за другом в строгой последовательности
    size_t offset = 0;
    for (const auto& parserEntry : ParsersPrivate::kParsers) {
        Tracy_ZoneScopedN("parserFunc");
        Tracy_ZoneText(parserEntry.name.data(), parserEntry.name.size());

        std::string_view blockName(reinterpret_cast<const char*>(&fileData[offset]), 8);
        assert(blockName == parserEntry.name);
        offset += 8;

        uint32_t blockSize = *reinterpret_cast<const uint32_t*>(&fileData[offset]);
        offset += sizeof(uint32_t);

        std::span<const uint8_t> block(fileData.data() + offset, fileData.data() + offset + blockSize);
        parserEntry.parser(block, data);
        offset += blockSize;
    }
    return true;
}

bool LVL_Parser::save(std::string_view lvlPath, const LVL_Data& data, std::string* error)
{
    Tracy_ZoneScoped;
    using namespace IoUtils;

    std::vector<uint8_t> saveData;
    saveData.reserve(32768); // 32KB

    writeString(saveData, ParsersPrivate::kParsers[BLK_LVER].name);
    writeUInt32(saveData, 4);
    writeUInt16(saveData, data.version.minor);
    writeUInt16(saveData, data.version.major);

    writeString(saveData, ParsersPrivate::kParsers[BLK_MPSZ].name);
    writeUInt32(saveData, 8);
    writeUInt32(saveData, data.mapSize.pixelWidth);
    writeUInt32(saveData, data.mapSize.pixelHeight);

    writeString(saveData, ParsersPrivate::kParsers[BLK_MHDR].name);
    writeUInt32(saveData, 8 + (data.mapTiles.chunks.size() * 4 * 6));
    writeUInt32(saveData, data.mapTiles.chunkWidth);
    writeUInt32(saveData, data.mapTiles.chunkHeight);
    for (const auto& chunk : data.mapTiles.chunks) {
        for (const auto& tile : chunk) {
            writeUInt16(saveData, tile.relief);
            writeUInt16(saveData, tile.sound);
            writeUInt16(saveData, tile.mask);
        }
    }

    writeString(saveData, ParsersPrivate::kParsers[BLK_MDSC].name);
    writeUInt32(saveData, 4 + (data.maskDescriptions.size() * 16));
    writeUInt32(saveData, data.maskDescriptions.size());
    for (const auto& mask : data.maskDescriptions) {
        writeUInt32(saveData, 0);
        writeUInt32(saveData, mask.number);
        writeUInt32(saveData, mask.x);
        writeUInt32(saveData, mask.y);
    }


    auto writeStructuredBlock = [&saveData] (int index, const std::vector<LVL_Description>& descriptions) {
        writeString(saveData, ParsersPrivate::kParsers[index].name);
        uint32_t stringSizes = 0;
        for (const auto& desc : descriptions) {
            stringSizes += 4 + desc.name.size();
        }
        writeUInt32(saveData, 4 + (descriptions.size() * 16) + stringSizes);
        writeUInt32(saveData, descriptions.size());
        for (const auto& desc : descriptions) {
            writeUInt16(saveData, desc.param1);
            writeUInt16(saveData, desc.param2);
            writeUInt32(saveData, desc.number);
            writeInt32(saveData, desc.position.x);
            writeInt32(saveData, desc.position.y);
            writeSizeAndString(saveData, desc.name);
        }
    };

    writeStructuredBlock(BLK_SDSC, data.staticDescriptions);
    writeStructuredBlock(BLK_ADSC, data.animationDescriptions);
    writeStructuredBlock(BLK_TDSC, data.triggerDescriptions);

    writeString(saveData, ParsersPrivate::kParsers[BLK_CGRP].name);
    uint32_t cellsSizes = 0;
    for (const auto& cellGroup : data.cellGroups) {
        cellsSizes += 4 + cellGroup.name.size() + 4 + (cellGroup.cells.size() * 4);
    }
    writeUInt32(saveData, 4 + cellsSizes);
    writeUInt32(saveData, data.cellGroups.size());
    for (const auto& cellGroup : data.cellGroups) {
        writeSizeAndString(saveData, cellGroup.name);
        writeUInt32(saveData, cellGroup.cells.size());
        for (const auto& cell : cellGroup.cells) {
            writeUInt16(saveData, cell.x);
            writeUInt16(saveData, cell.y);
        }
    }

    writeString(saveData, ParsersPrivate::kParsers[BLK_SENV].name);
    uint32_t soundPathSizes = 0;
    for (const auto& sound : data.sounds.otherSounds) {
        soundPathSizes += 4 + sound.path.size();
    }
    writeUInt32(saveData, 20 + 12 + data.sounds.levelTheme.size() + data.sounds.dayAmbience.size() + data.sounds.nightAmbience.size()
                           + soundPathSizes + (data.sounds.otherSounds.size() * 48));
    writeInt32(saveData, data.sounds.header.param1);
    writeFloat(saveData, data.sounds.header.param2);
    writeFloat(saveData, data.sounds.header.param3);
    writeFloat(saveData, data.sounds.header.param4);

    writeUInt32(saveData, data.sounds.otherSounds.size());

    writeSizeAndString(saveData, data.sounds.levelTheme);
    writeSizeAndString(saveData, data.sounds.dayAmbience);
    writeSizeAndString(saveData, data.sounds.nightAmbience);

    for (const auto& sound : data.sounds.otherSounds) {
        writeSizeAndString(saveData, sound.path);
        writeFloat(saveData, sound.chunkPositionX);
        writeFloat(saveData, sound.chunkPositionY);
        writeFloat(saveData, sound.param03);
        writeFloat(saveData, sound.param04);
        writeFloat(saveData, sound.param05);
        writeFloat(saveData, sound.param06);
        writeFloat(saveData, sound.param07);
        writeFloat(saveData, sound.param08);
        writeUInt32(saveData, sound.param09);
        writeUInt32(saveData, sound.param10);
        writeUInt32(saveData, sound.param11);
        writeUInt32(saveData, sound.param12);
    }

    writeString(saveData, ParsersPrivate::kParsers[BLK_WTHR].name);
    writeUInt32(saveData, 4);
    writeUInt16(saveData, data.weather.type);
    writeUInt16(saveData, data.weather.intensity);

    writeString(saveData, ParsersPrivate::kParsers[BLK_DOOR].name);
    uint32_t doorsSizes = 0;
    for (const auto& door : data.doors) {
        doorsSizes += 4 + door.sefName.size();
        doorsSizes += 4 + door.openAction.size();
        doorsSizes += 4 + door.closeAction.size();
        doorsSizes += 4 + door.cellGroup.size();
        doorsSizes += 4 + door.param1.size();
        doorsSizes += 4 + door.staticName.size();
    }
    writeUInt32(saveData, 4 + doorsSizes);
    writeUInt32(saveData, data.doors.size());
    for (const auto& door : data.doors) {
        writeSizeAndString(saveData, door.sefName);
        writeSizeAndString(saveData, door.openAction);
        writeSizeAndString(saveData, door.closeAction);
        writeSizeAndString(saveData, door.cellGroup);
        writeSizeAndString(saveData, door.param1);
        writeSizeAndString(saveData, door.staticName);
    }

    writeString(saveData, ParsersPrivate::kParsers[BLK_LFLS].name);
    writeUInt32(saveData, 4);
    writeUInt32(saveData, data.levelFloors);

    return FileUtils::saveFile(lvlPath, saveData, error);
}

void LVL_Parser::parseVersion(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);
    data.version.minor = *reinterpret_cast<const uint16_t*>(&block[0]);
    data.version.major = *reinterpret_cast<const uint16_t*>(&block[2]);
}

void LVL_Parser::parseMapSize(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 8);
    data.mapSize.pixelWidth = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.mapSize.pixelHeight = *reinterpret_cast<const uint32_t*>(&block[4]);
}

void LVL_Parser::parseMapTiles(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 8);
    data.mapTiles.chunkWidth  = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.mapTiles.chunkHeight = *reinterpret_cast<const uint32_t*>(&block[4]);

    size_t offset = 8;
    const size_t nChunks = data.mapTiles.chunkWidth * data.mapTiles.chunkHeight;
    assert(block.size() == offset + nChunks * 6 * 4);
    data.mapTiles.chunks.reserve(nChunks);
    for (size_t i = 0; i < nChunks; ++i) {
        MapChunk chunk;
        for (size_t t = 0; t < chunk.size(); ++t) {
            MapTile tile;
            tile.relief = *reinterpret_cast<const uint16_t*>(&block[offset]);
            tile.sound  = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
            tile.mask   = *reinterpret_cast<const uint16_t*>(&block[offset + 4]);
            chunk[t] = std::move(tile);
            offset += 6;
        }
        data.mapTiles.chunks.push_back(std::move(chunk));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseMaskDescriptions(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 4);
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.maskDescriptions.reserve(count);

    size_t offset = sizeof(uint32_t);
    assert(block.size() == offset + count * 16);
    for (size_t i = 0; i < count; ++i) {
        MaskDescription mask;
        offset += 4; // skip padding
        mask.number = *reinterpret_cast<const uint32_t*>(&block[offset]);
        mask.x      = *reinterpret_cast<const uint32_t*>(&block[offset + 4]);
        mask.y      = *reinterpret_cast<const uint32_t*>(&block[offset + 8]);
        offset += 12;
        data.maskDescriptions.push_back(std::move(mask));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseStaticDescriptions(std::span<const uint8_t> block, LVL_Data& data) {
    return parseStructuredBlock(block, data.staticDescriptions);
}

void LVL_Parser::parseAnimationDescriptions(std::span<const uint8_t> block, LVL_Data& data) {
    return parseStructuredBlock(block, data.animationDescriptions);
}

void LVL_Parser::parseTriggerDescriptions(std::span<const uint8_t> block, LVL_Data& data) {
    return parseStructuredBlock(block, data.triggerDescriptions);
}

void LVL_Parser::parseCellGroups(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 4);
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.cellGroups.reserve(count);

    size_t offset = sizeof(uint32_t);
    for (size_t i = 0; i < count; ++i) {
        CellGroup cellGroup;
        cellGroup.name = StringUtils::readStringWithLength(block, offset);

        uint32_t groupSize = *reinterpret_cast<const uint32_t*>(&block[offset]);
        cellGroup.cells.reserve(groupSize);
        offset += 4;
        for (uint32_t j = 0; j < groupSize; ++j) {
            TilePosition pos;
            pos.x = *reinterpret_cast<const uint16_t*>(&block[offset]);
            pos.y = *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
            offset += 4;
            cellGroup.cells.push_back(std::move(pos));
        }
        data.cellGroups.push_back(std::move(cellGroup));
    }

    assert(block.size() == offset);
}

void LVL_Parser::parseSounds(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 16);

    auto& sounds = data.sounds;

    sounds.header.param1 = *reinterpret_cast<const int32_t*>(&block[0]);
    sounds.header.param2 = *reinterpret_cast<const float*>(&block[4]);
    sounds.header.param3 = *reinterpret_cast<const float*>(&block[8]);
    sounds.header.param4 = *reinterpret_cast<const float*>(&block[12]);
    size_t offset = 16;

    uint32_t soundCount = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;

    sounds.levelTheme = StringUtils::readStringWithLength(block, offset);
    sounds.dayAmbience = StringUtils::readStringWithLength(block, offset);
    sounds.nightAmbience = StringUtils::readStringWithLength(block, offset);

    sounds.otherSounds.reserve(soundCount);
    for (uint32_t i = 0; i < soundCount; ++i) {
        ExtraSound extraSound;
        extraSound.path = StringUtils::readStringWithLength(block, offset);

        for (int p = 0; p < 8; ++p)
            reinterpret_cast<float*>(&extraSound.chunkPositionX)[p] = *reinterpret_cast<const float*>(&block[offset + p * 4]);
        offset += 32;
        extraSound.param09  = *reinterpret_cast<const uint32_t*>(&block[offset]);
        extraSound.param10 = *reinterpret_cast<const uint32_t*>(&block[offset + 4]);
        extraSound.param11 = *reinterpret_cast<const uint32_t*>(&block[offset + 8]);
        extraSound.param12 = *reinterpret_cast<const uint32_t*>(&block[offset + 12]);
        offset += 16;
        sounds.otherSounds.push_back(std::move(extraSound));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseWeather(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);
    data.weather.type = *reinterpret_cast<const uint16_t*>(&block[0]);
    data.weather.intensity = *reinterpret_cast<const uint16_t*>(&block[2]);
}

void LVL_Parser::parseDoors(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 4);
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.doors.reserve(count);

    size_t offset = sizeof(uint32_t);
    for (uint32_t i = 0; i < count; ++i) {
        Door door;
        door.sefName = StringUtils::readStringWithLength(block, offset);
        door.openAction = StringUtils::readStringWithLength(block, offset);
        door.closeAction = StringUtils::readStringWithLength(block, offset);
        door.cellGroup = StringUtils::readStringWithLength(block, offset);
        door.param1 = StringUtils::readStringWithLength(block, offset);
        door.staticName = StringUtils::readStringWithLength(block, offset);
        data.doors.push_back(std::move(door));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseLevelFloors(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);
    data.levelFloors = *reinterpret_cast<const uint32_t*>(&block[0]);
}

void LVL_Parser::parseStructuredBlock(std::span<const uint8_t> block, std::vector<LVL_Description>& data) {
    assert(block.size() >= 4);
    uint32_t count = *reinterpret_cast<const uint32_t*>(&block[0]);
    data.reserve(count);

    size_t offset = sizeof(uint32_t);
    for (size_t i = 0; i < count; ++i) {
        LVL_Description desc;
        desc.param1 =     *reinterpret_cast<const uint16_t*>(&block[offset]);
        desc.param2 =     *reinterpret_cast<const uint16_t*>(&block[offset + 2]);
        desc.number =     *reinterpret_cast<const uint32_t*>(&block[offset + 4]);
        desc.position.x = *reinterpret_cast<const  int32_t*>(&block[offset + 8]);
        desc.position.y = *reinterpret_cast<const  int32_t*>(&block[offset + 12]);
        offset += 16;
        desc.name = StringUtils::readStringWithLength(block, offset);
        data.push_back(std::move(desc));
    }
    assert(block.size() == offset);
}
