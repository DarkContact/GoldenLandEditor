#include "Level.h"

#include <format>

#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"

Level::Level(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType) {
    Tracy_ZoneScopedN("Level loading");
    m_data.name = level;

    std::string sefPath = levelSef(rootDirectory, level, levelType);
    SEF_Parser sefParser(sefPath);
    m_data.sefData = sefParser.data();

    std::string lvlPath = levelLvl(rootDirectory, m_data.sefData.pack);
    LVL_Parser lvlParser(lvlPath);
    m_data.lvlData = lvlParser.parse();

    std::string bgPath = levelBackground(rootDirectory, m_data.sefData.pack);
    TextureLoader::loadTextureFromFile(bgPath.c_str(), renderer, m_data.background);

    std::string minimapPath = levelMinimap(rootDirectory, m_data.sefData.pack);
    TextureLoader::loadTextureFromCsxFile(minimapPath.c_str(), renderer, m_data.minimap);
}

std::string Level::levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType) const
{
    return std::format("{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelType, level);
}

std::string Level::levelLvl(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{}/levels/lvl/{}.lvl", rootDirectory, levelPack);
}

std::string Level::levelBackground(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, levelPack);
}

std::string Level::levelMinimap(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{}/levels/pack/{}/bitmaps/minimap.csx", rootDirectory, levelPack);
}

const LevelData& Level::data() const
{
    return m_data;
}

LevelData& Level::data()
{
    return m_data;
}
