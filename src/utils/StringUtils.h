#pragma once
#include <cstdint>
#include <string>
#include <vector>

class StringUtils
{
public:
    StringUtils() = delete;

    static std::string toLower(std::string_view input);

    static std::string_view trimLeft(std::string_view input) noexcept;
    static std::string_view trimRight(std::string_view input) noexcept;
    static std::string_view trim(std::string_view input) noexcept;

    static int toInt(std::string_view input, int defaultValue = -1) noexcept;
    static bool parsePosition(std::string_view input, int& x, int& y) noexcept;

    static std::string_view extractQuotedValue(std::string_view line) noexcept;
    static std::string readStringWithLength(const std::vector<uint8_t>& block, size_t& offset);
    static std::string decodeWin1251ToUtf8(std::string_view input) noexcept;
};

