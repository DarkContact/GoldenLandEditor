#include "FileUtils.h"

#include <algorithm>
#include <cassert>
#include <format>

#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_timer.h"

#include "utils/TracyProfiler.h"

std::vector<uint8_t> FileUtils::loadFile(std::string_view filePath, std::string* error)
{
    Tracy_ZoneScoped;
    SDL_IOStream* stream = SDL_IOFromFile(filePath.data(), "rb");
    if (!stream) {
        if (error)
            *error = SDL_GetError();
        return {};
    }

    auto fileSize = SDL_GetIOSize(stream);
    if (fileSize < 0) {
        // Здесь возможно чтение чанками, пока не реализовано
        // https://github.com/libsdl-org/SDL/blob/79dae1b9d60cc69251380660aca2507c6c2df481/src/io/SDL_iostream.c#L1254
        SDL_CloseIO(stream);
        if (error)
            *error = SDL_GetError();
        return {};
    } else if (fileSize == 0) {
        SDL_CloseIO(stream);
        if (error)
            *error = "Empty file";
        return {};
    }

    int64_t bytesReadTotal = 0;
    std::vector<uint8_t> result(fileSize + 1);
    while (true) {
        auto bytesRead = SDL_ReadIO(stream, result.data() + bytesReadTotal, (size_t)(fileSize - bytesReadTotal));
        auto status = SDL_GetIOStatus(stream);
        if (status == SDL_IO_STATUS_NOT_READY) {
            SDL_Delay(1);
        } else if (status == SDL_IO_STATUS_ERROR) {
            SDL_CloseIO(stream);
            if (error)
                *error = SDL_GetError();
            return {};
        }

        bytesReadTotal += bytesRead;
        if (bytesReadTotal == fileSize) {
            break;
        }
    }
    result[fileSize] = '\0';
    SDL_CloseIO(stream);

    assert(bytesReadTotal == fileSize);
    return result;
}

std::string normalizePathForWindows(std::string_view inputPath) {
    std::string fixed(inputPath);
    std::replace(fixed.begin(), fixed.end(), '/', '\\');
    return fixed;
}

bool FileUtils::openFolder(std::string_view path, std::string* error)
{
    std::string command = std::format("explorer \"{}\"", normalizePathForWindows(path));
    int rc = system(command.c_str());
    if (rc != 0 && error) {
        *error = std::format("Open folder error. (Code: {}, Command: {})", rc, command);
    }
    return rc == 0;
}
