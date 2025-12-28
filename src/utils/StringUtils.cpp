#include "StringUtils.h"

#include <string_view>
#include <algorithm>
#include <charconv>
#include <cstring>
#include <cassert>

using namespace std::literals::string_view_literals;

static constexpr bool isSpace(int ch) noexcept {
    return (ch == ' ' || (ch >= '\t' && ch <= '\r'));
}


static constexpr char toLower(char c) noexcept {
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    return c;
}

static constexpr std::string_view win1251_to_utf8[] = {
    "\x00"sv, "\x01"sv, "\x02"sv, "\x03"sv, "\x04"sv, "\x05"sv, "\x06"sv, "\x07"sv, "\x08"sv, "\x09"sv, "\x0A"sv, "\x0B"sv, "\x0C"sv, "\x0D"sv, "\x0E"sv, "\x0F"sv,
    "\x10"sv, "\x11"sv, "\x12"sv, "\x13"sv, "\x14"sv, "\x15"sv, "\x16"sv, "\x17"sv, "\x18"sv, "\x19"sv, "\x1A"sv, "\x1B"sv, "\x1C"sv, "\x1D"sv, "\x1E"sv, "\x1F"sv,
    " "sv, "!"sv, "\""sv, "#"sv, "$"sv, "%"sv, "&"sv, "'"sv, "("sv, ")"sv, "*"sv, "+"sv, ","sv, "-"sv, "."sv, "/"sv,   // 0x20 - 0x2F
    "0"sv, "1"sv, "2"sv, "3"sv, "4"sv, "5"sv, "6"sv, "7"sv, "8"sv, "9"sv, ":"sv, ";"sv, "<"sv, "="sv, ">"sv, "?"sv,    // 0x30 - 0x3F
    "@"sv, "A"sv, "B"sv, "C"sv, "D"sv, "E"sv, "F"sv, "G"sv, "H"sv, "I"sv, "J"sv, "K"sv, "L"sv, "M"sv, "N"sv, "O"sv,    // 0x40 - 0x4F
    "P"sv, "Q"sv, "R"sv, "S"sv, "T"sv, "U"sv, "V"sv, "W"sv, "X"sv, "Y"sv, "Z"sv, "["sv, "\\sv", "]"sv, "^"sv, "_"sv,   // 0x50 - 0x5F
    "`"sv, "a"sv, "b"sv, "c"sv, "d"sv, "e"sv, "f"sv, "g"sv, "h"sv, "i"sv, "j"sv, "k"sv, "l"sv, "m"sv, "n"sv, "o"sv,    // 0x60 - 0x6F
    "p"sv, "q"sv, "r"sv, "s"sv, "t"sv, "u"sv, "v"sv, "w"sv, "x"sv, "y"sv, "z"sv, "{"sv, "|"sv, "}"sv, "~"sv, "\x7F"sv, // 0x70 - 0x7F

    "Ђ"sv, "Ѓ"sv, "‚"sv, "ѓ"sv, "„"sv, "…"sv, "†"sv, "‡"sv, "€"sv, "‰"sv, "Љ"sv, "‹"sv, "Њ"sv, "Ќ"sv, "Ћ"sv, "Џ"sv,    // 0x80 - 0x8F
    "ђ"sv, "‘"sv, "’"sv, "“"sv, "”"sv, "•"sv, "–"sv, "—"sv, "?"sv, "™"sv, "љ"sv, "›"sv, "њ"sv, "ќ"sv, "ћ"sv, "џ"sv,    // 0x90 - 0x9F
    " "sv, "Ў"sv, "ў"sv, "Ј"sv, "¤"sv, "Ґ"sv, "¦"sv, "§"sv, "Ё"sv, "©"sv, "Є"sv, "«"sv, "¬"sv, "\xAD"sv, "®", "Ї"sv,   // 0xA0 - 0xAF
    "°"sv, "±"sv, "І"sv, "і"sv, "ґ"sv, "µ"sv, "¶"sv, "·"sv, "ё"sv, "№"sv, "є"sv, "»"sv, "ј"sv, "Ѕ"sv, "ѕ"sv, "ї"sv,    // 0xB0 - 0xBF
    "А"sv, "Б"sv, "В"sv, "Г"sv, "Д"sv, "Е"sv, "Ж"sv, "З"sv, "И"sv, "Й"sv, "К"sv, "Л"sv, "М"sv, "Н"sv, "О"sv, "П"sv,    // 0xC0 - 0xCF
    "Р"sv, "С"sv, "Т"sv, "У"sv, "Ф"sv, "Х"sv, "Ц"sv, "Ч"sv, "Ш"sv, "Щ"sv, "Ъ"sv, "Ы"sv, "Ь"sv, "Э"sv, "Ю"sv, "Я"sv,    // 0xD0 - 0xDF
    "а"sv, "б"sv, "в"sv, "г"sv, "д"sv, "е"sv, "ж"sv, "з"sv, "и"sv, "й"sv, "к"sv, "л"sv, "м"sv, "н"sv, "о"sv, "п"sv,    // 0xE0 - 0xEF
    "р"sv, "с"sv, "т"sv, "у"sv, "ф"sv, "х"sv, "ц"sv, "ч"sv, "ш"sv, "щ"sv, "ъ"sv, "ы"sv, "ь"sv, "э"sv, "ю"sv, "я"sv,    // 0xF0 - 0xFF
};

void StringUtils::toLowerAscii(std::string_view input, std::span<char> output) noexcept {
    assert(output.size() >= input.size());
    std::transform(input.begin(), input.end(), output.begin(), toLower);
}

std::string_view StringUtils::trimLeft(std::string_view input) noexcept {
    while (!input.empty() && isSpace(input.front())) {
        input.remove_prefix(1);
    }
    return input;
}

std::string_view StringUtils::trimRight(std::string_view input) noexcept {
    while (!input.empty() && isSpace(input.back())) {
        input.remove_suffix(1);
    }
    return input;
}

std::string_view StringUtils::trim(std::string_view input) noexcept {
    return trimRight(trimLeft(input));
}

int StringUtils::toInt(std::string_view input, int defaultValue) noexcept
{
    int result = 0;
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

    if (ec != std::errc()) {
        return defaultValue;
    }

    return result;
}

uint32_t StringUtils::toUInt(std::string_view input, uint32_t defaultValue) noexcept
{
    uint32_t result = 0;
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

    if (ec != std::errc()) {
        return defaultValue;
    }

    return result;
}

double StringUtils::toDouble(std::string_view input, double defaultValue) noexcept
{
    double result = 0.0;
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), result);

    if (ec != std::errc()) {
        return defaultValue;
    }

    return result;
}

std::string_view StringUtils::eraseOneLineComment(std::string_view line) noexcept
{
    std::size_t commentPos = line.find("//");
    if (commentPos != std::string_view::npos) {
        return line.substr(0, commentPos);
    }
    return line;
}

bool StringUtils::parsePosition(std::string_view input, uint16_t& x, uint16_t& y) noexcept {
    auto spacePos = input.find(' ');
    if (spacePos == std::string_view::npos)
        return false;

    std::string_view xStr = trimRight(input.substr(0, spacePos));
    std::string_view yStr = trimLeft(input.substr(spacePos + 1));

    auto [px, ecx] = std::from_chars(xStr.data(), xStr.data() + xStr.size(), x);
    auto [py, ecy] = std::from_chars(yStr.data(), yStr.data() + yStr.size(), y);

    return ecx == std::errc() && ecy == std::errc();
}

std::string_view StringUtils::extractQuotedValue(std::string_view line) noexcept {
    size_t firstQuote = line.find('"');
    size_t lastQuote  = line.rfind('"');

    if (firstQuote != std::string_view::npos && lastQuote != std::string_view::npos && firstQuote < lastQuote) {
        return line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }

    return {};
}

std::string StringUtils::decodeWin1251ToUtf8(std::string_view input) noexcept {
    std::string result;
    for (unsigned char ch : input) {
        result += win1251_to_utf8[ch];
    }
    return result;
}

size_t StringUtils::decodeWin1251ToUtf8Buffer(std::string_view input, std::span<char> buffer) noexcept {
    assert(!buffer.empty());

    size_t pos = 0;
    for (unsigned char ch : input) {
        const std::string_view utf8View = win1251_to_utf8[ch];
        size_t len = utf8View.length();

        // Проверка на достаточность места
        assert(pos + len <= buffer.size() - 1);

        std::memcpy(buffer.data() + pos, utf8View.data(), len);
        pos += len;
    }
    buffer[pos] = '\0';
    return pos;
}

std::string_view StringUtils::filename(std::string_view path) noexcept
{
    auto pos = path.find_last_of("/\\");
    return (pos == std::string_view::npos) ? path
                                           : path.substr(pos + 1);
}

std::u8string_view StringUtils::toUtf8View(std::string_view input) noexcept {
    return {(char8_t*)input.data(), input.size()};
}

std::string_view StringUtils::fromUtf8View(std::u8string_view input) noexcept {
    return {(char*)input.data(), input.size()};
}
