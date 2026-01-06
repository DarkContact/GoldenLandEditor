#include "SDB_Parser.h"

#include <cassert>

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

using namespace IoUtils;

bool SDB_Parser::parse(std::string_view sdbPath, SDB_Data& data, std::string* error)
{
    Tracy_ZoneScoped;
    auto fileData = FileUtils::loadFile(sdbPath, error);
    if (fileData.empty()) {
        return false;
    }

    bool xorRequired = false;
    size_t offset = 0;
    if (readString(fileData, 4, offset) != "SDB ") {
        xorRequired = true;
        offset = 0;
    }

    constexpr size_t kStringTextSize = 16384;
    char stringText[kStringTextSize];
    while (offset < fileData.size()) {
        int32_t id = readInt32(fileData, offset);
        auto textSv = readStringWithSize(fileData, offset);
        assert((textSv.size() * 2) < kStringTextSize);

        if (xorRequired) {
            char decodedText[kStringTextSize];
            std::copy(textSv.begin(), textSv.end(), decodedText);
            decodedText[textSv.size()] = '\0';
            for (size_t i = 0; i < textSv.size(); ++i) {
                decodedText[i] ^= 0xAA;
            }
            StringUtils::decodeWin1251ToUtf8Buffer(decodedText, stringText);
        } else {
            StringUtils::decodeWin1251ToUtf8Buffer(textSv, stringText);
        }
        data.strings.emplace_hint(data.strings.end(), id, stringText);
    }

    assert(fileData.size() == offset);
    return true;
}
