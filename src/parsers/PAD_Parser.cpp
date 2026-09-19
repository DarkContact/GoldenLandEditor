#include "PAD_Parser.h"

#include <algorithm>
#include <cassert>

#include "utils/IoUtils.h"
#include "utils/DebugLog.h"
#include "utils/FileUtils.h"

std::optional<PAD_Data> PAD_Parser::parse(std::string_view path, std::string* error)
{
    using namespace IoUtils;

    auto fileData = FileUtils::loadFile(path, error);
    if (fileData.empty()) {
        return {};
    }

    // Проверка заголовка "PAD "
    size_t offset = 0;
    if (readString(fileData, 4, offset) != "PAD ") {
        if (error)
            *error = "Incorrect PAD file. Missing 'PAD '";
        return {};
    }

    std::optional<PAD_Data> result = PAD_Data();
    uint32_t dataSize = readUInt32(fileData, offset);
    uint32_t animationMasks = readUInt32(fileData, offset);

    assert(offset + dataSize <= fileData.size());
    while (offset < dataSize)
    {
        uint32_t animationType = readUInt32(fileData, offset);
        bool isValidMask = std::ranges::any_of(PAD_Data::typeMasks, [animationType](uint32_t x) { return x == animationType; });
        if (!isValidMask) {
            LogFmt("Invalid Mask: {}", animationType);
            break;
        }
        bool isCorrectMask = (animationMasks & animationType) != 0;
        if (!isCorrectMask) {
            LogFmt("Incorrect Mask: {}", animationType);
            break;
        }
        bool isAlreadyHaveMask = std::ranges::any_of(result->animations, [animationType](const PAD_Animation& x) { return x.type == animationType; });
        if (isAlreadyHaveMask) {
            LogFmt("Is already have mask: {}", animationType);
            break;
        }

        PAD_Animation animation;
        animation.type = static_cast<PAD_AnimationTypeMask>(animationType);
        LogFmt("Type: {}", animationTypeMaskToString(animation.type));
        int32_t offsetAnimationStart = offset;
        int32_t animationSize = readInt32(fileData, offset);

        size_t animationOffset = 4;
        animation.delay = readInt32(fileData, offset);
        animation.framesPerRow = readInt32(fileData, offset);
        animation.frameWidth = readInt32(fileData, offset);
        animation.frameHeight = readInt32(fileData, offset);
        animation.anchorX = readInt32(fileData, offset);
        animation.anchorY = readInt32(fileData, offset);
        animation.movementX = readFloat(fileData, offset);
        animation.movementY = readFloat(fileData, offset);

        uint32_t tableSize = readUInt32(fileData, offset);
        animation.rowCount = tableSize / 8 / animation.framesPerRow;
        animationOffset += 9 * 4;

        size_t endOffset = offsetAnimationStart + animationSize;

        animation.crops.reserve(animation.rowCount);
        for (uint32_t row = 0; row < animation.rowCount; ++row) {
            std::vector<PAD_CropsFrame> frames;
            frames.reserve(animation.framesPerRow);

            for (uint32_t f = 0; f < animation.framesPerRow; ++f) {
                PAD_CropsFrame frame;
                frame.x = readInt16(fileData, offset);
                frame.y = readInt16(fileData, offset);
                frame.width = readInt16(fileData, offset);
                frame.height = readInt16(fileData, offset);
                frames.emplace_back(std::move(frame));
            }

            animation.crops.emplace_back(std::move(frames));
            animationOffset += 8;

            assert(offset <= endOffset);
        }

        animation.shadowFrame.width = readInt32(fileData, offset);
        animation.shadowFrame.height = readInt32(fileData, offset);
        animation.shadowFrame.anchorX = readInt32(fileData, offset);
        animation.shadowFrame.anchorY = readInt32(fileData, offset);

        uint32_t shadowTableSize = readUInt32(fileData, offset);
        animation.shadowRowCount = shadowTableSize / 8 / animation.framesPerRow;
        LogFmt("ShadowTableSize: {}", shadowTableSize);
        LogFmt("ShadowRowCount: {}", animation.shadowRowCount);

        LogFmt("Offset: {}", offset);
        LogFmt("EndOffset: {}", endOffset);
        assert(offset <= endOffset);
        offset = endOffset;

        result->animations.push_back(std::move(animation));
    }

    // TODO: Разобраться что за данные в конце файла

    return result;
}
