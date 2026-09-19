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
        animationOffset += 8 * 4;
        while (animationOffset < animationSize) {
            uint16_t lVal = readInt16(fileData, offset);
            uint16_t rVal = readInt16(fileData, offset);
            animation.offsets.push_back({lVal, rVal});
            animationOffset += 4;
        }
        result->animations.push_back(std::move(animation));
    }

    // TODO: Разобраться что за данные в конце файла

    return result;
}
