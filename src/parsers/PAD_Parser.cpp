#include "PAD_Parser.h"

#include <cassert>

#include "utils/IoUtils.h"
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
    uint32_t size = readUInt32(fileData, offset);
    result->animationMask = readUInt32(fileData, offset);

    for (uint32_t i = 1; i != static_cast<uint32_t>(PAD_AnimationTypeMask::hits3); i <<= 1) {
        if ((result->animationMask & i) == 0) continue;
        result->animations.push_back(static_cast<PAD_AnimationTypeMask>(i));
    }

    assert(12 + size <= fileData.size());

    result->animationData.assign(fileData.begin() + 12,
                                 fileData.begin() + 12 + size);

    return result;
}
