#include "StringUtils.h"

#include <algorithm>

constexpr static bool isSpace(int ch) noexcept {
    return ((ch >= '\t' && ch <= '\r') || ch == ' ');
}

static const std::string win1251_to_utf8[] = {
    "Ђ", "Ѓ", "‚", "ѓ", "„", "…", "†", "‡", "€", "‰", "Љ", "‹", "Њ", "Ќ", "Ћ", "Џ",  // 0x80 - 0x8F
    "ђ", "‘", "’", "“", "”", "•", "–", "—", "?", "™", "љ", "›", "њ", "ќ", "ћ", "џ",  // 0x90 - 0x9F
    " ", "Ў", "ў", "Ј", "¤", "Ґ", "¦", "§", "Ё", "©", "Є", "«", "¬", "\xAD", "®", "Ї", // 0xA0 - 0xAF
    "°", "±", "І", "і", "ґ", "µ", "¶", "·", "ё", "№", "є", "»", "ј", "Ѕ", "ѕ", "ї",  // 0xB0 - 0xBF
    "А", "Б", "В", "Г", "Д", "Е", "Ж", "З", "И", "Й", "К", "Л", "М", "Н", "О", "П",  // 0xC0 - 0xCF
    "Р", "С", "Т", "У", "Ф", "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я",  // 0xD0 - 0xDF
    "а", "б", "в", "г", "д", "е", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п",  // 0xE0 - 0xEF
    "р", "с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я",  // 0xF0 - 0xFF
};

std::string StringUtils::toLower(std::string_view input) {
    std::string result(input);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::trim(std::string_view input) {
    auto begin = input.cbegin();
    auto end = input.cend();
    // skip white space from end
    while (begin < end && isSpace(end[-1])) {
        --end;
    }
    // skip white space from start
    while (begin < end && isSpace(*begin)) {
        begin++;
    }
    return {begin, end};
}

std::string StringUtils::extractQuotedValue(const std::string& line) {
    size_t firstQuote = line.find('"');
    size_t lastQuote  = line.rfind('"');

    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }

    return {};
}

std::string StringUtils::readStringWithLength(const std::vector<uint8_t>& block, size_t& offset) {
    if (offset + 4 > block.size()) return "";
    uint32_t len = *reinterpret_cast<const uint32_t*>(&block[offset]);
    offset += 4;
    if (offset + len > block.size()) return "";
    std::string s(reinterpret_cast<const char*>(&block[offset]), len);
    offset += len;
    return s;
}

std::string StringUtils::decodeWin1251ToUtf8(const std::string& input) {
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
