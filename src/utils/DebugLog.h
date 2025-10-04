#pragma once
#include <source_location>
#include <string_view>
#include <cassert>
#include <format>
#include <cstdio>

class DebugLog {
public:
    DebugLog() = delete;

    template<typename... Args>
    static void print(const std::format_string<Args...> fmt, Args&&... args) {
        // FIXME: Исправить std::source_location::current()
        // https://stackoverflow.com/a/57548488/14617664
        printFormatImpl(std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

    static void print(std::string_view msg, const std::source_location& location = std::source_location::current()) {
        toOutput(location, msg);
    }

private:
    template<typename... Args>
    static void printFormatImpl(const std::source_location& location, const std::format_string<Args...> fmt, Args&&... args) {
        try {
            constexpr size_t kBufferSize = 8192;
            constexpr size_t kBufferSizeWithoutZero = kBufferSize - 1;
            char buffer[kBufferSize];

            auto result = std::format_to_n(buffer, kBufferSizeWithoutZero, fmt, std::forward<Args>(args)...);
            // TODO: Как-то выводить, то что не влезло в буффер
            // if (kBufferSizeWithoutZero < result.size) {
            //     std::string message = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            //     toOutput(location, message);
            // }
            buffer[std::min((size_t)result.size, kBufferSizeWithoutZero)] = '\0';
            toOutput(location, buffer);
        } catch (const std::format_error& ex) {
            fputs(ex.what(), stderr);
        }
    }

    static void toOutput(const std::source_location& location, std::string_view msg) {
        fputs(msg.data(), stdout);

        constexpr size_t kBufferSize = 512;
        char bufferInfo[kBufferSize];
        auto result = std::format_to_n(bufferInfo, kBufferSize, " [{}:{}]\n", location.file_name(), location.line());
        assert(result.size < kBufferSize);
        bufferInfo[result.size] = '\0';
        fputs(bufferInfo, stdout);
    }
};
