#include "FileUtils.h"

#include <algorithm>
#include <cassert>
#include <format>

#include "SDL3/SDL_iostream.h"

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

    int64_t bytesReadedTotal = 0;
    std::vector<uint8_t> buffer(fileSize);
    uint8_t* pos = buffer.data();
    while (true) {
        auto bytesReaded = SDL_ReadIO(stream, pos, 1); // FIXME: есть проблемы при чтении по 1024, нужно разобраться
        auto status = SDL_GetIOStatus(stream);
        if (status == SDL_IO_STATUS_ERROR) {
            SDL_CloseIO(stream);
            if (error)
                *error = SDL_GetError();
            return {};
        }
        bytesReadedTotal += bytesReaded;

        if (status == SDL_IO_STATUS_EOF) break;
        pos += bytesReaded;
    }
    SDL_CloseIO(stream);

    assert(bytesReadedTotal == fileSize);
    return buffer;
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
