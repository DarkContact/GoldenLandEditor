#include "FileUtils.h"

#include <fstream>

#include "utils/TracyProfiler.h"

std::vector<uint8_t> FileUtils::loadFile(std::string_view filePath, std::string* error)
{
    Tracy_ZoneScoped;
    std::ifstream in(filePath.data(), std::ios::binary | std::ios::ate);
    if (!in && error) {
        *error = "Can't open file";
        return {};
    }

    auto size = in.tellg();
    if (size == 0 && error) {
        *error = "Empty file";
        return {};
    }

    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    in.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}
