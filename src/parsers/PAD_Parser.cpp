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
    uint32_t animationSize = readUInt32(fileData, offset);
    result->animationMasks = readUInt32(fileData, offset);

    assert(offset + animationSize <= fileData.size());
    while (offset < animationSize)
    {
        LogFmt("offset: {}", offset);

        uint32_t animationType = readUInt32(fileData, offset);
        bool isValidMask = std::ranges::any_of(PAD_Data::typeMasks, [animationType](uint32_t x) { return x == animationType; });
        if (!isValidMask) {
            LogFmt("Invalid Mask: {}", animationType);
            break;
        }
        bool isCorrectMask = (result->animationMasks & animationType) != 0;
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
        animation.size = readInt32(fileData, offset);
        LogFmt("animation.size: {}", animation.size);

        size_t animationOffset = 4;
        animation.delay = readInt32(fileData, offset);
        animation.framesPerRow = readInt32(fileData, offset);
        animation.width = readInt32(fileData, offset);
        animation.height = readInt32(fileData, offset);
        animation.anchorX = readInt32(fileData, offset);
        animation.anchorY = readInt32(fileData, offset);
        animation.movementX = readFloat(fileData, offset);
        animation.movementY = readFloat(fileData, offset);
        animation.p11 = readInt32(fileData, offset);
        animationOffset += 9 * 4;
        while (animationOffset < animation.size) {
            uint16_t lVal = readInt16(fileData, offset);
            uint16_t rVal = readInt16(fileData, offset);
            animation.offsets.push_back({lVal, rVal});
            animationOffset += 4;
        }
        result->animations.push_back(std::move(animation));
    }

    // Как будто ничего полезного
    offset = 12 + animationSize;
    if (offset < fileData.size()) {
        result->endSize = readInt32(fileData, offset);
    }

    if (result->endSize > 0) {
        result->endData.assign(fileData.begin() + 12 + animationSize + 4,
                               fileData.begin() + 12 + animationSize + 4 + result->endSize);
    }

    return result;
}
