#include "Level.h"

#include <filesystem>
#include <algorithm>
#include <cassert>
#include <format>

#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

std::optional<Level> Level::loadLevel(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view level, std::string_view levelType, std::string* error)
{
    Tracy_ZoneScoped;
    LogFmt("Loading level: {}", level);
    assert(error);

    Level levelObj;
    levelObj.m_rootDirectory = rootDirectory;

    auto& levelData = levelObj.m_data;
    levelData.name = level;

    std::string sefPath = levelSef(rootDirectory, level, levelType);
    if (!SEF_Parser::parse(sefPath, levelData.sefData, error)) {
        LogFmt("Loading .sef failed. {}", *error);
        return {};
    }

    std::string lvlPath = levelLvl(rootDirectory, levelData.sefData.pack);
    if (!LVL_Parser::parse(lvlPath, levelData.lvlData, error)) {
        LogFmt("Loading .lvl failed. {}", *error);
        return {};
    }

    std::string sdbPath = levelSdb(rootDirectory, level, levelType);
    if (!SDB_Parser::parse(sdbPath, levelData.sdbData, error)) {
        LogFmt("Loading .sdb failed. {}", *error);
        // Допустимо, если не загрузилось, но нужно уведомить пользователя
    }

    std::string bgPath = levelBackground(rootDirectory, levelData.sefData.pack);
    TextureLoader::loadTextureFromFile(bgPath.c_str(), renderer, levelData.background);

    std::string minimapPath = levelMinimap(rootDirectory, levelData.sefData.pack);
    TextureLoader::loadTextureFromCsxFile(minimapPath.c_str(), renderer, levelData.minimap);

    std::string laoPath = levelLao(rootDirectory, levelData.sefData.pack);
    if (std::filesystem::exists(StringUtils::utf8View(laoPath))) {
        std::string laoError;
        levelData.laoData = LAO_Parser::parse(laoPath, &laoError);
        if (!levelData.laoData) {
            LogFmt("Loading .lao failed. {}", laoError); // Допустимо
        }
    }

    // Валидация анимаций
    int animationDescCount = levelData.lvlData.animationDescriptions.size();
    int animationLaoCount = levelData.laoData ? levelData.laoData->infos.size() : 0;
    int animationFilesCount = 0;
    std::string levelAnimationDirPath = levelAnimationDir(rootDirectory, levelData.sefData.pack);
    if (std::filesystem::exists(StringUtils::utf8View(levelAnimationDirPath))) {
        animationFilesCount = std::distance(std::filesystem::directory_iterator(StringUtils::utf8View(levelAnimationDirPath)),
                                            std::filesystem::directory_iterator{});
    }
    bool animationOk = animationDescCount == animationLaoCount && animationLaoCount == animationFilesCount;
    if (!animationOk) {
        LogFmt("Animation counts mismatch (animationDescCount: {}, animationLaoCount: {}, animationFilesCount: {})", animationDescCount, animationLaoCount, animationFilesCount);
    }

    int minimalSize = std::min(std::min(animationDescCount, animationLaoCount), animationFilesCount);
    std::span<LVL_Description> animationDescriptionView(levelData.lvlData.animationDescriptions.data(), minimalSize);

    // Отсортируем описание анимаций
    if (animationDescCount >= 2) {
        std::sort(animationDescriptionView.begin(),
                  animationDescriptionView.end(),
                  [] (const LVL_Description& left, const LVL_Description& right) {
                      return left.number < right.number;
                  });
    }

    for (int i = 0; i < minimalSize; ++i) {
        std::string levelAnimationPath = levelAnimation(rootDirectory, levelData.sefData.pack, i);
        LevelAnimation animation(levelData.lvlData.animationDescriptions.at(i));
        animation.delayMs = levelData.laoData->infos[i].delay;
        if (!TextureLoader::loadHeightAnimationFromCsxFile(levelAnimationPath, levelData.laoData->infos[i].height, renderer, animation.textures, error)) {
            LogFmt("Loading texture for animation failed. {}", *error);
            return {};
        }
        levelData.animations.push_back(std::move(animation));
    }

    return std::make_optional(std::move(levelObj));
}

std::string Level::levelSef(std::string_view rootDirectory, std::string_view level, std::string_view levelType)
{
    return std::format("{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelType, level);
}

std::string Level::levelSdb(std::string_view rootDirectory, std::string_view level, std::string_view levelType)
{
    return std::format("{0}/levels/{1}/{2}/{2}.sdb", rootDirectory, levelType, level);
}

std::string Level::levelLvl(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/lvl/{}.lvl", rootDirectory, levelPack);
}

std::string Level::levelBackground(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/pack/{}/bitmaps/layer.jpg", rootDirectory, levelPack);
}

std::string Level::levelMinimap(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/pack/{}/bitmaps/minimap.csx", rootDirectory, levelPack);
}

std::string Level::levelLao(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{0}/levels/pack/{1}/data/animated/{1}.lao", rootDirectory, levelPack);
}

std::string Level::levelAnimationDir(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/pack/{}/bitmaps/animated", rootDirectory, levelPack);
}

std::string Level::levelAnimation(std::string_view rootDirectory, std::string_view levelPack, int index)
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

std::string Level::levelDir(std::string_view levelType) const
{
    return std::format("{}/levels/{}/{}", m_rootDirectory, levelType, m_data.name);
}

std::string Level::levelPackDir() const
{
    return std::format("{}/levels/pack/{}", m_rootDirectory, m_data.sefData.pack);
}
