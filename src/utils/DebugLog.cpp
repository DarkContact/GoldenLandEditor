#include "DebugLog.h"

#include <cassert>

#include "TracyProfiler.h"

void DebugLog::toOutput(const std::source_location& location, std::string_view msg) {
    Tracy_Message(msg.data(), msg.size());

    fputs(msg.data(), stdout);

    constexpr size_t kBufferSize = 512;
    char bufferInfo[kBufferSize];
    auto result = std::format_to_n(bufferInfo, kBufferSize, " [{}:{}]\n", location.file_name(), location.line());
    assert(result.size < kBufferSize);
    bufferInfo[result.size] = '\0';
    fputs(bufferInfo, stdout);
}
