#pragma once
#include <cstdint>
#include <string>
#include <span>

template<typename T>
concept LineCallback = requires(T callback, std::string_view stringView) {
    { callback(stringView) } -> std::same_as<void>;
};

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

    template <LineCallback Callback>
    static void forEachLine(std::string_view buffer, Callback&& callback) noexcept;

    static std::string_view filename(std::string_view path) noexcept;
    static std::u8string_view utf8View(std::string_view input) noexcept;
};



template<LineCallback Callback>
inline void StringUtils::forEachLine(std::string_view buffer, Callback&& callback) noexcept {
    size_t start = 0;
    const size_t length = buffer.size();

    for (size_t i = 0; i < length - 1; ++i) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            size_t lineLength = i - start;
            callback(buffer.substr(start, lineLength));

            // Переносы строк windows (\r\n) и двойной перенос (\n\n)
            if (buffer[i + 1] == '\n') {
                ++i;
            }

            start = i + 1;
        }
    }

    if (start < length) {
        callback(buffer.substr(start));
    }
}
