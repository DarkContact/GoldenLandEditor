#include "MDF_Parser.h"

#include <cstring>
#include <cassert>
#include <span>

#include "utils/FileUtils.h"
#include "utils/StringUtils.h"

int32_t readInt32(std::span<const uint8_t> fileData, size_t& offset) {
    int32_t result = *reinterpret_cast<const int32_t*>(&fileData[offset]);
    offset += sizeof(int32_t);
    return result;
}

float readFloat(std::span<const uint8_t> fileData, size_t& offset) {
    float result = *reinterpret_cast<const float*>(&fileData[offset]);
    offset += sizeof(float);
    return result;
}

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
    result->totalDurationMs = readInt32(fileData, offset);

    for (int i = 0; i < MDF_Parser::kMaxLayers; ++i) {
        int32_t animationCount = readInt32(fileData, offset);
        if (animationCount > 0) {
            MDF_Layer layer;
            layer.animations.reserve(animationCount);
            for (int i = 0; i < animationCount; ++i) {
                MDF_Animation anim;
                anim.framesCount = readInt32(fileData, offset);
                anim.xOffset = readInt32(fileData, offset);
                anim.yOffset = readInt32(fileData, offset);
                anim.a04 = readInt32(fileData, offset);
                anim.a05 = readInt32(fileData, offset);
                anim.startTimeMs = readInt32(fileData, offset);
                anim.endTimeMs = readInt32(fileData, offset);
                anim.maskAnimationPath = StringUtils::readStringWithLength(fileData, offset);
                anim.animationPath = StringUtils::readStringWithLength(fileData, offset);

                int32_t paramsCount = readInt32(fileData, offset);
                anim.params.reserve(paramsCount);
                for (int p = 0; p < paramsCount; ++p) {
                    MDF_Params params;
                    params.p01 = readInt32(fileData, offset);
                    params.p02 = readInt32(fileData, offset);
                    params.flags = readInt32(fileData, offset);
                    params.nFrame = readInt32(fileData, offset);
                    params.p05 = readInt32(fileData, offset);
                    params.p06 = readFloat(fileData, offset);
                    params.p07 = readFloat(fileData, offset);
                    params.p08 = readFloat(fileData, offset);
                    params.p09 = readFloat(fileData, offset);
                    params.animationTimeMs = readInt32(fileData, offset);
                    anim.params.push_back(std::move(params));
                }
                layer.animations.push_back(std::move(anim));
            }
            result->layers.push_back(std::move(layer));
        }
    }
    int32_t eof = readInt32(fileData, offset);

    assert(eof == 1);
    assert(fileData.size() == offset);
    return result;
}
