#include "DebugLog.h"

#include <cassert>
#ifdef DEBUG_LOG_TIMESTAMP
  #include <chrono>
#endif

#include "TracyProfiler.h"
#ifdef DEBUG_LOG_CONTEXT
  #include "StringUtils.h"
#endif

void DebugLog::toOutput(const std::source_location& location, std::string_view msg) {
#ifdef DEBUG_LOG_TIMESTAMP
    // Время в формате UTC: [10:30:25.367]
    using namespace std::chrono;
    auto now = system_clock::now();
    constexpr size_t kBufferNowSize = 24;
    char bufferNow[kBufferNowSize];
    auto resultNow = std::format_to_n(bufferNow, kBufferNowSize, "[{:%T}]: ", time_point_cast<milliseconds>(now));
    assert(resultNow.size < kBufferNowSize);
    bufferNow[resultNow.size] = '\0';
    fputs(bufferNow, stdout);
#endif

    // Сообщение
    Tracy_Message(msg.data(), msg.size());
    fputs(msg.data(), stdout);

#ifdef DEBUG_LOG_CONTEXT
    // Информация о расположении в исходниках: [main.cpp:35]
    constexpr size_t kBufferInfoSize = 280;
    char bufferInfo[kBufferInfoSize];

    std::string_view fileNameOnly = StringUtils::filename(location.file_name()); // Достаём только имя файла
    auto resultInfo = std::format_to_n(bufferInfo, kBufferInfoSize, " [{}:{}]", fileNameOnly, location.line());
    assert(resultInfo.size < kBufferInfoSize);
    bufferInfo[resultInfo.size] = '\0';
    fputs(bufferInfo, stdout);
#endif

    fputc('\n', stdout);
    fflush(stdout);
}
