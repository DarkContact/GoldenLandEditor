#pragma once
#include <string_view>
#include <cstdint>
#include <vector>

class FileLoader
{
public:
    FileLoader() = delete;

    static std::vector<uint8_t> loadFile(std::string_view filePath, std::string* error = nullptr);
};
