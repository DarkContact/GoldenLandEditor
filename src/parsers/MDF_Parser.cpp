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
    result->height = readInt32(fileData, offset);

    for (int i = 0; i < MDF_Parser::kMaxEntries; ++i) {
        int32_t packCount = readInt32(fileData, offset);
        if (packCount > 0) {
            MDF_Entry entry;
            for (int i = 0; i < packCount; ++i) {
                MDF_Pack pack;
                pack.framesCount = readInt32(fileData, offset);
                pack.a02 = readInt32(fileData, offset);
                pack.a03 = readInt32(fileData, offset);
                pack.a04 = readInt32(fileData, offset);
                pack.a05 = readInt32(fileData, offset);
                pack.a06 = readInt32(fileData, offset);
                pack.a07 = readInt32(fileData, offset);
                pack.maskAnimationPath = StringUtils::readStringWithLength(fileData, offset);
                pack.animationPath = StringUtils::readStringWithLength(fileData, offset);

                int32_t paramsCount = readInt32(fileData, offset);
                for (int p = 0; p < paramsCount; ++p) {
                    MDF_Params params;
                    params.p01 = readInt32(fileData, offset);
                    params.p02 = readInt32(fileData, offset);
                    params.p03 = readInt32(fileData, offset);
                    params.p04 = readInt32(fileData, offset);
                    params.p05 = readInt32(fileData, offset);
                    params.p06 = readFloat(fileData, offset);
                    params.p07 = readInt32(fileData, offset);
                    params.p08 = readInt32(fileData, offset);
                    params.p09 = readInt32(fileData, offset);
                    params.p10 = readInt32(fileData, offset);
                    pack.params.push_back(std::move(params));
                }
                entry.packs.push_back(std::move(pack));
            }
            result->entries.push_back(std::move(entry));
        }
    }
    int32_t eof = readInt32(fileData, offset);

    assert(eof == 1);
    assert(fileData.size() == offset);
    return result;
}
