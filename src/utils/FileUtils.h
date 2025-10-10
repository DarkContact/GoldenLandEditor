#pragma once
#include <string_view>
#include <cstdint>
#include <string>
#include <vector>

class FileUtils
{
public:
    FileUtils() = delete;

    static std::vector<uint8_t> loadFile(std::string_view filePath, std::string* error = nullptr);
};
