#include "LAO_Parser.h"

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"

using namespace IoUtils;

std::optional<LAO_Data> LAO_Parser::parse(std::string_view laoPath, std::string* error = nullptr)
{
    auto fileData = FileUtils::loadFile(laoPath, error);
    if (fileData.empty()) {
        return {};
    }

    if (fileData.size() % 8 != 0) {
        if (error)
            *error = "Incorrect data";
        return {};
    }

    std::optional<LAO_Data> result = LAO_Data();

    size_t offset = 0;
    while (offset < fileData.size()) {
        uint32_t height = readUInt32(fileData, offset);
        uint32_t duration = readUInt32(fileData, offset);

        result->infos.emplace_back(height, duration);
    }
    return result;
}
