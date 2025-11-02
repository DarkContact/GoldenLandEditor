#include "SDB_Parser.h"

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

bool SDB_Parser::parse(std::string_view sdbPath, SDB_Data& data, std::string* error)
{
    Tracy_ZoneScoped;
    auto fileData = FileUtils::loadFile(sdbPath, error);
    if (fileData.empty()) {
        return false;
    }

    bool xorRequired = false;
    size_t offset = 4;
    if (std::memcmp(fileData.data(), "SDB ", 4) != 0) {
        xorRequired = true;
        offset = 0;
    }

    while (offset < fileData.size()) {
        int32_t id = IoUtils::readInt32(fileData, offset);

        auto textSv = StringUtils::readStringWithLength(fileData, offset);
        std::string text = std::string(textSv);
        if (xorRequired) {
            for (auto& byte : text) {
                byte ^= 0xAA;
            }
        }

        text = StringUtils::decodeWin1251ToUtf8(text);
        data.strings[id] = text;
    }

    assert(fileData.size() == offset);
    return true;
}
