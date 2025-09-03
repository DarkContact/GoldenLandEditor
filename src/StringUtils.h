#pragma once
#include <cstdint>
#include <string>
#include <vector>

class StringUtils
{
public:
    StringUtils() = delete;

    static std::string toLower(std::string_view input);

    static std::string trim(std::string_view input);

    static std::string extractQuotedValue(const std::string& line);
    static std::string readStringWithLength(const std::vector<uint8_t>& block, size_t& offset);
    static std::string decodeWin1251ToUtf8(const std::string& input);
};

