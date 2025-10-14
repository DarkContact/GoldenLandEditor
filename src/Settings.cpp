#include "Settings.h"

#include <cassert>
#include <array>

#include "ini.h"

using namespace std::literals::string_view_literals;

struct SettingEntry {
    Setting id;
    SettingType type;
    std::string_view group;
    std::string_view key;
};

consteval auto makeSettings() {
    return std::to_array<SettingEntry>({
        { Setting::kFontFilepath, SettingType::kString, "fonts"sv,      "filepath"sv },
        { Setting::kFontSize,     SettingType::kInt,    "fonts"sv,      "size"sv     },
        { Setting::kRootDir,      SettingType::kString, "resources"sv,  "root_dir"sv }
    });
}

struct Settings::Data {
    Data(std::string_view path) :
        settingsFile(path) {}


    mINI::INIFile settingsFile;
    mINI::INIStructure settingsValues;
    static constexpr auto settingsMap = makeSettings();
};

Settings::Settings(std::string_view path) :
    d(std::make_unique<Data>(path))
{
    readFromFile();
}

Settings::~Settings()
{
    saveToFile();
}

void Settings::readFromFile() {
    d->settingsFile.read(d->settingsValues);
}

void Settings::saveToFile() {
    d->settingsFile.write(d->settingsValues);
}

std::string Settings::readString(Setting setting, std::string_view defaultValue) const {
    const auto& mapping = d->settingsMap[static_cast<int>(setting)];
    assert(mapping.type == SettingType::kString);

    auto result = d->settingsValues[std::string(mapping.group)][std::string(mapping.key)];
    return result.empty() ? std::string(defaultValue) : result;
}

void Settings::writeString(Setting setting, std::string_view value) {
    const auto& mapping = d->settingsMap[static_cast<int>(setting)];
    assert(mapping.type == SettingType::kString);

    d->settingsValues[std::string(mapping.group)][std::string(mapping.key)] = value;
}

int Settings::readInt(Setting setting, int defaultValue) const {
    const auto& mapping = d->settingsMap[static_cast<int>(setting)];
    assert(mapping.type == SettingType::kInt);

    auto result = d->settingsValues[std::string(mapping.group)][std::string(mapping.key)];
    return result.empty() ? defaultValue : std::stoi(result);
}

void Settings::writeInt(Setting setting, int value) {
    const auto& mapping = d->settingsMap[static_cast<int>(setting)];
    assert(mapping.type == SettingType::kInt);

    d->settingsValues[std::string(mapping.group)][std::string(mapping.key)] = std::to_string(value);
}
