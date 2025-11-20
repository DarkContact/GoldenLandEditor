#include "StringUtils.h"

#include <charconv>
#include <cassert>

constexpr static bool isSpace(int ch) noexcept {
    return (ch == ' ' || (ch >= '\t' && ch <= '\r'));
}

static constexpr const char* win1251_to_utf8[] = {
    "Ђ", "Ѓ", "‚", "ѓ", "„", "…", "†", "‡", "€", "‰", "Љ", "‹", "Њ", "Ќ", "Ћ", "Џ",  // 0x80 - 0x8F
    "ђ", "‘", "’", "“", "”", "•", "–", "—", "?", "™", "љ", "›", "њ", "ќ", "ћ", "џ",  // 0x90 - 0x9F
    " ", "Ў", "ў", "Ј", "¤", "Ґ", "¦", "§", "Ё", "©", "Є", "«", "¬", "\xAD", "®", "Ї", // 0xA0 - 0xAF
    "°", "±", "І", "і", "ґ", "µ", "¶", "·", "ё", "№", "є", "»", "ј", "Ѕ", "ѕ", "ї",  // 0xB0 - 0xBF
    "А", "Б", "В", "Г", "Д", "Е", "Ж", "З", "И", "Й", "К", "Л", "М", "Н", "О", "П",  // 0xC0 - 0xCF
    "Р", "С", "Т", "У", "Ф", "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я",  // 0xD0 - 0xDF
    "а", "б", "в", "г", "д", "е", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п",  // 0xE0 - 0xEF
    "р", "с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я",  // 0xF0 - 0xFF
};

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
    for (unsigned char c : input) {
        if (c < 0x80) {
           result += c;
        } else {
           result += win1251_to_utf8[c - 0x80];
        }
    }
    return result;
}

std::string_view StringUtils::filename(std::string_view path) noexcept
{
    auto pos = path.find_last_of("/\\");
    return (pos == std::string_view::npos) ? path
                                           : path.substr(pos + 1);
}

std::u8string_view StringUtils::utf8View(std::string_view input) noexcept {
    return {(char8_t*)input.data(), input.size()};
}
