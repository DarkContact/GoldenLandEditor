#include "Level.h"

#include <filesystem>
#include <algorithm>
#include <format>

#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/DebugLog.h"

Level::Level(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType) :
    m_rootDirectory(rootDirectory)
{
    Tracy_ZoneScopedN("Level loading");
    LogFmt("Level loading: {}", level);
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

    std::string laoPath = levelLao(rootDirectory, m_data.sefData.pack);
    if (std::filesystem::exists(laoPath)) {
        std::string error;
        m_data.laoData = LAO_Parser::parse(laoPath, &error);
        if (!m_data.laoData) {
            LogFmt("Loading .lao failed. {}", error);
        }
    }

    // Валидация анимаций
    int animationDescCount = m_data.lvlData.animationDescriptions.size();
    int animationLaoCount = m_data.laoData ? m_data.laoData->infos.size() : 0;
    int animationFilesCount = 0;
    std::string levelAnimationDirPath = levelAnimationDir(rootDirectory, m_data.sefData.pack);
    if (std::filesystem::exists(levelAnimationDirPath)) {
        animationFilesCount = std::distance(std::filesystem::directory_iterator(levelAnimationDirPath), std::filesystem::directory_iterator{});
    }
    bool animationOk = animationDescCount == animationLaoCount && animationLaoCount == animationFilesCount;
    if (!animationOk) {
        LogFmt("Animation counts mismatch (animationDescCount: {}, animationLaoCount: {}, animationFilesCount: {})", animationDescCount, animationLaoCount, animationFilesCount);
    }

    std::span<LVL_Description> animationDescriptionView(m_data.lvlData.animationDescriptions.data(),
                                                        std::min((size_t)animationLaoCount, m_data.lvlData.animationDescriptions.size()));

    // Отсортируем описание анимаций
    std::sort(animationDescriptionView.begin(),
              animationDescriptionView.end(),
              [] (const LVL_Description& left, const LVL_Description& right) {
        return left.number < right.number;
    });

    if (m_data.laoData && !animationDescriptionView.empty()) {
        for (int i = 0; i < m_data.laoData->infos.size(); ++i) {
            std::string levelAnimationPath = levelAnimation(rootDirectory, m_data.sefData.pack, i);
            if (std::filesystem::exists(levelAnimationPath)) {
                Animation animation(m_data.lvlData.animationDescriptions.at(i));
                animation.duration = m_data.laoData->infos[i].duration;
                std::string error;
                if (!TextureLoader::loadFixedHeightTexturesFromCsxFile(levelAnimationPath, m_data.laoData->infos[i].height, renderer, animation.textures, &error)) {
                    LogFmt("TextureLoader::loadFixedHeightTexturesFromCsxFile failed. {}", error);
                }
                m_data.animations.push_back(std::move(animation));
            } else {
                break;
            }
        }
    }
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

std::string Level::levelLao(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{0}/levels/pack/{1}/data/animated/{1}.lao", rootDirectory, levelPack);
}

std::string Level::levelAnimationDir(std::string_view rootDirectory, std::string_view levelPack) const
{
    return std::format("{}/levels/pack/{}/bitmaps/animated", rootDirectory, levelPack);
}

std::string Level::levelAnimation(std::string_view rootDirectory, std::string_view levelPack, int index) const
{
    return std::format("{}/levels/pack/{}/bitmaps/animated/anim_{}.csx", rootDirectory, levelPack, index);
}

const LevelData& Level::data() const
{
    return m_data;
}

LevelData& Level::data()
{
    return m_data;
}

std::string Level::levelPackDir() const
{
    return std::format("{}/levels/pack/{}", m_rootDirectory, m_data.sefData.pack);
}
