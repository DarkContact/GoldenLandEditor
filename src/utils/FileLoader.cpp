#include "FileLoader.h"

#include <fstream>
#include <format>

#include "utils/TracyProfiler.h"

std::vector<uint8_t> FileLoader::loadFile(std::string_view m_filePath)
{
    Tracy_ZoneScoped;
    std::ifstream in(m_filePath.data(), std::ios::binary | std::ios::ate);
    if (!in)
        throw std::runtime_error(std::format("Cannot open file: {}", m_filePath));
    auto size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(size);
    in.read(reinterpret_cast<char*>(buf.data()), size);
    return buf;
}
