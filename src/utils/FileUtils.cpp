#include "FileUtils.h"

#include <algorithm>
#include <fstream>
#include <format>

#include "utils/TracyProfiler.h"

std::vector<uint8_t> FileUtils::loadFile(std::string_view filePath, std::string* error)
{
    Tracy_ZoneScoped;
    std::ifstream in(filePath.data(), std::ios::binary | std::ios::ate);
    if (!in) {
        if (error)
            *error = "Can't open file";
        return {};
    }

    auto size = in.tellg();
    if (size == 0) {
        if (error)
            *error = "Empty file";
        return {};
    }

    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    in.read(reinterpret_cast<char*>(buffer.data()), size);
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
