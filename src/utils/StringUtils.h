#pragma once
#include <cstdint>
#include <cassert>
#include <string>
#include <format>
#include <span>

template<typename T>
concept LineCallback = requires(T callback, std::string_view stringView) {
    { callback(stringView) } -> std::same_as<void>;
};

class StringUtils
{
public:
    StringUtils() = delete;

    static void toLowerAscii(std::string_view input, std::span<char> output) noexcept;

    static std::string_view trimLeft(std::string_view input) noexcept;
    static std::string_view trimRight(std::string_view input) noexcept;
    static std::string_view trim(std::string_view input) noexcept;

    static int toInt(std::string_view input, int defaultValue = -1) noexcept;
    static uint32_t toUInt(std::string_view input, uint32_t defaultValue = -1) noexcept;
    static double toDouble(std::string_view input, double defaultValue = -1.0) noexcept;

    static std::string_view eraseOneLineComment(std::string_view line) noexcept;
    static bool parsePosition(std::string_view input, uint16_t& x, uint16_t& y) noexcept;

    static std::string_view extractQuotedValue(std::string_view line) noexcept;
    static std::string decodeWin1251ToUtf8(std::string_view input) noexcept;
    static size_t decodeWin1251ToUtf8Buffer(std::string_view input, std::span<char> buffer) noexcept;

    template <LineCallback Callback>
    static void forEachLine(std::string_view buffer, Callback&& callback) noexcept;

    static std::string_view filename(std::string_view path) noexcept;
    static std::u8string_view toUtf8View(std::string_view input) noexcept;
    static std::string_view fromUtf8View(std::u8string_view input) noexcept;

    template <typename... Args>
    static size_t formatToBuffer(std::span<char> buffer, std::format_string<Args...> fmt, Args&&... args);
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

// #include "utils/Platform.h"

template<typename... Args>
inline size_t StringUtils::formatToBuffer(std::span<char> buffer, std::format_string<Args...> fmt, Args&&... args) {
    // static_assert(!BX_COMPILER_GCC || (BX_COMPILER_GCC >= 130300),
    //               "std::format_to_n have bug. Please update GCC version. More info: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110990");

    const size_t bufferSize = buffer.size();
    assert(bufferSize);

    auto result = std::format_to_n(buffer.data(),
                                   bufferSize - 1,
                                   fmt,
                                   std::forward<Args>(args)...);
    *result.out = '\0';

    assert(result.size < (bufferSize - 1));
    return result.size;
}
