#include "Level.h"

#include <format>

bool Level::loadLevel(std::string_view rootDirectory, std::string_view level, std::string_view levelType) {
    std::string sefPath = levelSef(rootDirectory, level, levelType);

    SEF_Parser sefParser(sefPath);

    m_data.sefData = sefParser.data();

    return true;
}

std::string Level::levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const
{
    return std::format("{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelType, level);
}

LevelData Level::data() const
{
    return m_data;
}
