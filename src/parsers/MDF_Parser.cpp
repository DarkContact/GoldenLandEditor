#include "MDF_Parser.h"

#include <cstring>
#include <cassert>
#include <span>

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"

std::optional<MDF_Data> MDF_Parser::parse(std::string_view path, std::string* error)
{
    auto fileData = FileUtils::loadFile(path, error);
    if (fileData.empty()) {
        return {};
    }

    // Проверка заголовка "MDF "
    if (std::memcmp(fileData.data(), "MDF ", 4) != 0) {
        if (error)
            *error = "Incorrect MDF file. Missing 'MDF '";
        return {};
    }
    size_t offset = 4;

    std::optional<MDF_Data> result = MDF_Data();
    result->totalDurationMs = IoUtils::readInt32(fileData, offset);

    for (int i = 0; i < MDF_Parser::kMaxLayers; ++i) {
        int32_t animationCount = IoUtils::readInt32(fileData, offset);
        if (animationCount > 0) {
            MDF_Layer layer;
            layer.animations.reserve(animationCount);
            for (int i = 0; i < animationCount; ++i) {
                MDF_Animation anim;
                anim.framesCount = IoUtils::readInt32(fileData, offset);
                anim.xOffset = IoUtils::readInt32(fileData, offset);
                anim.yOffset = IoUtils::readInt32(fileData, offset);
                anim.a04 = IoUtils::readInt32(fileData, offset);
                anim.isReverse = IoUtils::readInt32(fileData, offset);
                anim.startTimeMs = IoUtils::readInt32(fileData, offset);
                anim.endTimeMs = IoUtils::readInt32(fileData, offset);
                anim.maskAnimationPath = StringUtils::readStringWithLength(fileData, offset);
                anim.animationPath = StringUtils::readStringWithLength(fileData, offset);

                int32_t paramsCount = IoUtils::readInt32(fileData, offset);
                anim.params.reserve(paramsCount);
                for (int p = 0; p < paramsCount; ++p) {
                    MDF_Params params;
                    params.p01 = IoUtils::readInt32(fileData, offset);
                    params.p02 = IoUtils::readInt32(fileData, offset);
                    params.flags = IoUtils::readInt32(fileData, offset);
                    params.nFrame = IoUtils::readInt32(fileData, offset);
                    params.p05 = IoUtils::readInt32(fileData, offset);
                    params.delayMs = IoUtils::readFloat(fileData, offset);
                    params.alpha = IoUtils::readFloat(fileData, offset);
                    params.p08 = IoUtils::readFloat(fileData, offset);
                    params.p09 = IoUtils::readFloat(fileData, offset);
                    params.animationTimeMs = IoUtils::readInt32(fileData, offset);
                    anim.params.push_back(std::move(params));
                }
                layer.animations.push_back(std::move(anim));
            }
            result->layers.push_back(std::move(layer));
        }
    }
    int32_t eof = IoUtils::readInt32(fileData, offset);

    assert(eof == 1);
    assert(fileData.size() == offset);
    return result;
}
