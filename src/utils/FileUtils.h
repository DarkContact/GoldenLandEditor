#pragma once
#include <string_view>
#include <cstdint>
#include <string>
#include <vector>
#include <span>

class FileUtils
{
public:
    FileUtils() = delete;

    static std::vector<uint8_t> loadFile(std::string_view filePath, std::string* error = nullptr);
    static bool saveFile(std::string_view filePath, std::span<const uint8_t> fileData, std::string* error = nullptr);

    static std::vector<uint8_t> loadJpegPhotoshopThumbnail(std::string_view filePath, std::string* error = nullptr);

    static bool openFolderAndSelectItems(std::string_view path, std::span<const std::string_view> files, std::string* error = nullptr);
};
