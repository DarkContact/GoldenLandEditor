#pragma once
#include <cstdint>
#include <string>
#include <span>

class StringUtils
{
public:
    StringUtils() = delete;

    static std::string toLower(std::string_view input);

    static std::string_view trimLeft(std::string_view input) noexcept;
    static std::string_view trimRight(std::string_view input) noexcept;
    static std::string_view trim(std::string_view input) noexcept;

    static int toInt(std::string_view input, int defaultValue = -1) noexcept;
    static bool parsePosition(std::string_view input, uint16_t& x, uint16_t& y) noexcept;

    static std::string_view extractQuotedValue(std::string_view line) noexcept;
    static std::string_view readStringWithLength(std::span<const uint8_t> block, size_t& offset) noexcept;
    static std::string decodeWin1251ToUtf8(std::string_view input) noexcept;

    static std::u8string_view utf8View(std::string_view input) noexcept;
};

