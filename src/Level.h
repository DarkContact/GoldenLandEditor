#pragma once
#include <string_view>

#include "parsers/SEF_Parser.h"

struct LevelData {
    SEF_Data sefData;
};

class Level
{
public:
    bool loadLevel(std::string_view rootDirectory, std::string_view level, std::string_view levelType = "single");

    LevelData data() const;

private:
    std::string levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const;

    LevelData m_data;
};
