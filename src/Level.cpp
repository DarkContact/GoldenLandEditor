#include "Level.h"

#include <format>

#include "TextureLoader.h"
#include "StringUtils.h"

bool Level::loadLevel(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType) {
    m_data.name = level;

    std::string sefPath = levelSef(rootDirectory, level, levelType);

    SEF_Parser sefParser(sefPath);
    m_data.sefData = sefParser.data();

    std::string bgPath = levelBackground(rootDirectory, StringUtils::toLower(m_data.sefData.pack));
    TextureLoader::loadTextureFromFile(bgPath.c_str(), renderer, &m_data.background);

    return true;
}

std::string Level::levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const
{
    return std::format("{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelType, level);
}

std::string Level::levelBackground(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, levelPack);
}

LevelData Level::data() const
{
    return m_data;
}
