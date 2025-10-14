#pragma once
#include <string_view>
#include <cstdint>
#include <memory>
#include <string>

enum class Setting : uint16_t {
    kFontFilepath,
    kFontSize,
    kRootDir
};

enum class SettingType : uint16_t {
    kString,
    kBool,
    kInt
};

class Settings {
    struct Data;

public:
    Settings(std::string_view path);
    ~Settings();

    std::string readString(Setting setting, std::string_view defaultValue = {}) const;
    void writeString(Setting setting, std::string_view value);

    int readInt(Setting setting, int defaultValue = 0) const;
    void writeInt(Setting setting, int value);

private:
    void readFromFile();
    void saveToFile();

    std::unique_ptr<Data> d;
};
