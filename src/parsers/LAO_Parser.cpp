#include "LAO_Parser.h"

#include <cstring>

#include "utils/FileLoader.h"

std::optional<LAO_Data> LAO_Parser::parse(std::string_view laoPath, std::string* error = nullptr)
{
    auto fileData = FileLoader::loadFile(laoPath, error);
    if (fileData.empty()) {
        return {};
    }

    if (fileData.size() % 8 != 0) {
        if (error)
            *error = "Incorrect data";
        return {};
    }

    size_t offset = 0;

    std::optional<LAO_Data> result = LAO_Data();
    while (offset < fileData.size()) {
        uint32_t height;
        std::memcpy(&height, &fileData[offset], sizeof(uint32_t));
        offset += 4;

        uint32_t duration;
        std::memcpy(&duration, &fileData[offset], sizeof(uint32_t));
        offset += 4;

        result->infos.emplace_back(height, duration);
    }
    return result;
}
