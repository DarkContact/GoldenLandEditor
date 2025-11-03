#include "LVL_Parser.h"

#include <cassert>

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/TracyProfiler.h"

using namespace IoUtils;
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

        std::string_view blockName = readString(fileData, 8, offset);
        assert(blockName == parserEntry.name);

        uint32_t blockSize = readUInt32(fileData, offset);

        std::span<const uint8_t> block(fileData.data() + offset,
                                       fileData.data() + offset + blockSize);
        parserEntry.parser(block, data);
        offset += blockSize;
    }
    return true;
}

bool LVL_Parser::save(std::string_view lvlPath, const LVL_Data& data, std::string* error)
{
    Tracy_ZoneScoped;

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
            writeStringWithSize(saveData, desc.name);
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
        writeStringWithSize(saveData, cellGroup.name);
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

    writeStringWithSize(saveData, data.sounds.levelTheme);
    writeStringWithSize(saveData, data.sounds.dayAmbience);
    writeStringWithSize(saveData, data.sounds.nightAmbience);

    for (const auto& sound : data.sounds.otherSounds) {
        writeStringWithSize(saveData, sound.path);
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
        writeStringWithSize(saveData, door.sefName);
        writeStringWithSize(saveData, door.openAction);
        writeStringWithSize(saveData, door.closeAction);
        writeStringWithSize(saveData, door.cellGroup);
        writeStringWithSize(saveData, door.param1);
        writeStringWithSize(saveData, door.staticName);
    }

    writeString(saveData, ParsersPrivate::kParsers[BLK_LFLS].name);
    writeUInt32(saveData, 4);
    writeUInt32(saveData, data.levelFloors);

    return FileUtils::saveFile(lvlPath, saveData, error);
}

void LVL_Parser::parseVersion(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);

    size_t offset = 0;
    data.version.minor = readUInt16(block, offset);
    data.version.major = readUInt16(block, offset);
    assert(block.size() == offset);
}

void LVL_Parser::parseMapSize(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 8);

    size_t offset = 0;
    data.mapSize.pixelWidth = readUInt32(block, offset);
    data.mapSize.pixelHeight = readUInt32(block, offset);
    assert(block.size() == offset);
}

void LVL_Parser::parseMapTiles(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 8);

    size_t offset = 0;
    data.mapTiles.chunkWidth  = readUInt32(block, offset);
    data.mapTiles.chunkHeight = readUInt32(block, offset);

    const size_t nChunks = data.mapTiles.chunkWidth * data.mapTiles.chunkHeight;
    assert(block.size() == offset + nChunks * 6 * 4);
    data.mapTiles.chunks.reserve(nChunks);
    for (size_t i = 0; i < nChunks; ++i) {
        MapChunk chunk;
        for (size_t t = 0; t < chunk.size(); ++t) {
            MapTile tile;
            tile.relief = readUInt16(block, offset);
            tile.sound = readUInt16(block, offset);
            tile.mask = readUInt16(block, offset);
            chunk[t] = std::move(tile);
        }
        data.mapTiles.chunks.push_back(std::move(chunk));
    }

    assert(block.size() == offset);
}

void LVL_Parser::parseMaskDescriptions(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 4);

    size_t offset = 0;
    uint32_t count = readUInt32(block, offset);
    data.maskDescriptions.reserve(count);

    assert(block.size() == offset + count * 16);
    for (size_t i = 0; i < count; ++i) {
        MaskDescription mask;
        offset += 4; // skip padding
        mask.number = readUInt32(block, offset);
        mask.x = readUInt32(block, offset);
        mask.y = readUInt32(block, offset);
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

    size_t offset = 0;
    uint32_t count = readUInt32(block, offset);
    data.cellGroups.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        CellGroup cellGroup;
        cellGroup.name = readStringWithSize(block, offset);

        uint32_t groupSize = readUInt32(block, offset);
        cellGroup.cells.reserve(groupSize);
        for (uint32_t j = 0; j < groupSize; ++j) {
            TilePosition pos;
            pos.x = readUInt16(block, offset);
            pos.y = readUInt16(block, offset);
            cellGroup.cells.push_back(std::move(pos));
        }
        data.cellGroups.push_back(std::move(cellGroup));
    }

    assert(block.size() == offset);
}

void LVL_Parser::parseSounds(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 16);

    auto& sounds = data.sounds;

    size_t offset = 0;
    sounds.header.param1 = readInt32(block, offset);
    sounds.header.param2 = readFloat(block, offset);
    sounds.header.param3 = readFloat(block, offset);
    sounds.header.param4 = readFloat(block, offset);

    uint32_t soundCount = readUInt32(block, offset);

    sounds.levelTheme = readStringWithSize(block, offset);
    sounds.dayAmbience = readStringWithSize(block, offset);
    sounds.nightAmbience = readStringWithSize(block, offset);

    sounds.otherSounds.reserve(soundCount);
    for (uint32_t i = 0; i < soundCount; ++i) {
        ExtraSound extraSound;
        extraSound.path = readStringWithSize(block, offset);
        extraSound.chunkPositionX = readFloat(block, offset);
        extraSound.chunkPositionY = readFloat(block, offset);
        extraSound.param03 = readFloat(block, offset);
        extraSound.param04 = readFloat(block, offset);
        extraSound.param05 = readFloat(block, offset);
        extraSound.param06 = readFloat(block, offset);
        extraSound.param07 = readFloat(block, offset);
        extraSound.param08 = readFloat(block, offset);
        extraSound.param09 = readUInt32(block, offset);
        extraSound.param10 = readUInt32(block, offset);
        extraSound.param11 = readUInt32(block, offset);
        extraSound.param12 = readUInt32(block, offset);
        sounds.otherSounds.push_back(std::move(extraSound));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseWeather(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);

    size_t offset = 0;
    data.weather.type = readUInt16(block, offset);
    data.weather.intensity = readUInt16(block, offset);
    assert(block.size() == offset);
}

void LVL_Parser::parseDoors(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() >= 4);

    size_t offset = 0;
    uint32_t count = readUInt32(block, offset);
    data.doors.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        Door door;
        door.sefName = readStringWithSize(block, offset);
        door.openAction = readStringWithSize(block, offset);
        door.closeAction = readStringWithSize(block, offset);
        door.cellGroup = readStringWithSize(block, offset);
        door.param1 = readStringWithSize(block, offset);
        door.staticName = readStringWithSize(block, offset);
        data.doors.push_back(std::move(door));
    }
    assert(block.size() == offset);
}

void LVL_Parser::parseLevelFloors(std::span<const uint8_t> block, LVL_Data& data) {
    assert(block.size() == 4);

    size_t offset = 0;
    data.levelFloors = readUInt32(block, offset);
    assert(block.size() == offset);
}

void LVL_Parser::parseStructuredBlock(std::span<const uint8_t> block, std::vector<LVL_Description>& data) {
    assert(block.size() >= 4);

    size_t offset = 0;
    uint32_t count = readUInt32(block, offset);
    data.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        LVL_Description desc;
        desc.param1 = readUInt16(block, offset);
        desc.param2 = readUInt16(block, offset);
        desc.number = readUInt32(block, offset);
        desc.position.x = readInt32(block, offset);
        desc.position.y = readInt32(block, offset);
        desc.name = readStringWithSize(block, offset);
        data.push_back(std::move(desc));
    }
    assert(block.size() == offset);
}
