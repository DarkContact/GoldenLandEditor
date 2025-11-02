#pragma once
#include <source_location>
#include <string_view>
#include <format>
#include <cstdio>

class DebugLog {
public:
    DebugLog() = delete;

    static void print(const std::source_location& location, std::string_view msg) {
        toOutput(location, msg);
    }

    template<typename... Args>
    static void printFmt(const std::source_location& location, const std::format_string<Args...> fmt, Args&&... args) {
        try {
            constexpr size_t kBufferSize = 8192;
            constexpr size_t kBufferSizeWithoutZero = kBufferSize - 1;
            char buffer[kBufferSize];

            auto result = std::format_to_n(buffer, kBufferSizeWithoutZero, fmt, std::forward<Args>(args)...);
            if (kBufferSizeWithoutZero < result.size) {
                buffer[kBufferSizeWithoutZero - 3] = '.';
                buffer[kBufferSizeWithoutZero - 2] = '.';
                buffer[kBufferSizeWithoutZero - 1] = '.';
            }
            buffer[std::min((size_t)result.size, kBufferSizeWithoutZero)] = '\0';
            toOutput(location, buffer);
        } catch (const std::format_error& ex) {
            fputs(ex.what(), stderr);
        }
    }

private:
    static void toOutput(const std::source_location& location, std::string_view msg);
};

#ifdef NDEBUG
  #define Log(msg)
  #define LogFmt(fmt, ...)
#else
  #define Log(msg)         DebugLog::print(std::source_location::current(), msg)
  #define LogFmt(fmt, ...) DebugLog::printFmt(std::source_location::current(), fmt, ##__VA_ARGS__)
#endif
