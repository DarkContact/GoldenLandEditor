#include "FileLoader.h"

#include <fstream>
#include <format>

#include "utils/TracyProfiler.h"

std::vector<uint8_t> FileLoader::loadFile(std::string_view filePath, std::string* error)
{
    Tracy_ZoneScoped;
    std::ifstream in(filePath.data(), std::ios::binary | std::ios::ate);
    if (!in && error) {
        *error = std::format("Can't open file: {}", filePath);
        return {};
    }

    auto size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    in.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}
