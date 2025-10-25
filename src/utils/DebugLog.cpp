#include "DebugLog.h"

#include <chrono>
#include <cassert>

#include "TracyProfiler.h"
#include "StringUtils.h"

void DebugLog::toOutput(const std::source_location& location, std::string_view msg) {
    Tracy_Message(msg.data(), msg.size());

    // Время в формате UTC: [10:30:25.367]
    using namespace std::chrono;
    auto now = system_clock::now();
    constexpr size_t kBufferNowSize = 24;
    char bufferNow[kBufferNowSize];
    auto resultNow = std::format_to_n(bufferNow, kBufferNowSize, "[{:%T}]: ", time_point_cast<milliseconds>(now));
    assert(resultNow.size < kBufferNowSize);
    bufferNow[resultNow.size] = '\0';
    fputs(bufferNow, stdout);

    // Сообщение
    fputs(msg.data(), stdout);

    // Достаём только имя файла
    std::string_view fileNameOnly = StringUtils::filename(location.file_name());

    // Информация о расположении в исходниках: [main.cpp:35]
    constexpr size_t kBufferInfoSize = 280;
    char bufferInfo[kBufferInfoSize];
    auto resultInfo = std::format_to_n(bufferInfo, kBufferInfoSize, " [{}:{}]\n", fileNameOnly, location.line());
    assert(resultInfo.size < kBufferInfoSize);
    bufferInfo[resultInfo.size] = '\0';
    fputs(bufferInfo, stdout);

    fflush(stdout);
}
