#include "Level.h"

#include <filesystem>
#include <algorithm>
#include <cassert>
#include <format>

#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

std::optional<Level> Level::loadLevel(SDL_Renderer* renderer, std::string_view rootDirectory, std::string_view levelName, LevelType levelType, std::string* error)
{
    Tracy_ZoneScoped;
    LogFmt("Loading level: {}", levelWindowName(levelName, levelType));
    assert(error);

    Level levelObj;
    auto& levelData = levelObj.m_data;
    levelData.name = levelName;
    levelData.type = levelType;
    auto levelTypeString = levelTypeToString(levelType);

    std::string sefPath = levelSef(rootDirectory, levelTypeString, levelName);
    if (!SEF_Parser::parse(sefPath, levelData.sefData, error)) {
        LogFmt("Loading .sef failed. {}", *error);
        return {};
    }

    std::string lvlPath = levelLvl(rootDirectory, levelData.sefData.pack);
    if (!LVL_Parser::parse(lvlPath, levelData.lvlData, error)) {
        LogFmt("Loading .lvl failed. {}", *error);
        return {};
    }

    std::string sdbPath = levelSdb(rootDirectory, levelTypeString, levelName);
    if (!SDB_Parser::parse(sdbPath, levelData.sdbData, error)) {
        LogFmt("Loading .sdb failed. {}", *error);
        // Допустимо не загрузить. Нужно уведомить пользователя
    }

    std::string bgPath = levelBackground(rootDirectory, levelData.sefData.pack);
    TextureLoader::loadTextureFromFile(bgPath.c_str(), renderer, levelData.background);

    std::string minimapPath = levelMinimap(rootDirectory, levelData.sefData.pack);
    TextureLoader::loadTextureFromCsxFile(minimapPath.c_str(), renderer, levelData.minimap);

    std::string laoPath = levelLao(rootDirectory, levelData.sefData.pack);
    if (std::filesystem::exists(StringUtils::toUtf8View(laoPath))) {
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
    if (std::filesystem::exists(StringUtils::toUtf8View(levelAnimationDirPath))) {
        animationFilesCount = std::distance(std::filesystem::directory_iterator(StringUtils::toUtf8View(levelAnimationDirPath)),
                                            std::filesystem::directory_iterator{});
    }
    bool animationOk = animationDescCount == animationLaoCount && animationLaoCount == animationFilesCount;
    if (!animationOk) {
        LogFmt("Animation counts mismatch (animationDescCount: {}, animationLaoCount: {}, animationFilesCount: {})", animationDescCount, animationLaoCount, animationFilesCount);
    }

    int minimalAnimationSize = std::min(std::min(animationDescCount, animationLaoCount), animationFilesCount);
    std::span<LVL_Description> animationDescriptionView(levelData.lvlData.animationDescriptions.data(), minimalAnimationSize);

    // Отсортируем описание анимаций
    if (animationDescCount >= 2) {
        std::sort(animationDescriptionView.begin(),
                  animationDescriptionView.end(),
                  [] (const LVL_Description& left, const LVL_Description& right) {
                      return left.number < right.number;
                  });
    }

    for (int i = 0; i < minimalAnimationSize; ++i) {
        std::string levelAnimationPath = levelAnimation(rootDirectory, levelData.sefData.pack, i);
        LevelAnimation animation(levelData.lvlData.animationDescriptions.at(i));
        animation.delayMs = levelData.laoData->infos[i].delay;
        if (!TextureLoader::loadHeightAnimationFromCsxFile(levelAnimationPath, levelData.laoData->infos[i].height, renderer, animation.textures, error)) {
            LogFmt("Loading texture for animation failed. {}", *error);
            return {};
        }
        levelData.animations.push_back(std::move(animation));
    }

    // Загрузка тригеров
    int triggerDescCount = levelData.lvlData.triggerDescriptions.size();
    int triggerFilesCount = 0;
    std::string levelTriggerDirPath = levelTriggerDir(rootDirectory, levelData.sefData.pack);
    if (std::filesystem::exists(StringUtils::toUtf8View(levelTriggerDirPath))) {
        triggerFilesCount = std::distance(std::filesystem::directory_iterator(StringUtils::toUtf8View(levelTriggerDirPath)),
                                            std::filesystem::directory_iterator{});
    }
    bool triggersOk = triggerFilesCount == triggerDescCount;
    if (!triggersOk) {
        LogFmt("Trigger counts mismatch (triggerDescCount: {}, triggerFilesCount: {})", triggerDescCount, triggerFilesCount);
    }

    int minimalTriggerSize = std::min(triggerDescCount, triggerFilesCount);
    std::span<LVL_Description> triggerDescriptionView(levelData.lvlData.triggerDescriptions.data(), minimalTriggerSize);

    // Отсортируем описание триггеров
    if (animationDescCount >= 2) {
        std::sort(triggerDescriptionView.begin(),
                  triggerDescriptionView.end(),
                  [] (const LVL_Description& left, const LVL_Description& right) {
                      return left.number < right.number;
                  });
    }

    for (int i = 0; i < minimalTriggerSize; ++i) {
        std::string levelTriggerPath = levelTrigger(rootDirectory, levelData.sefData.pack, i);
        Texture trigger;
        if (!TextureLoader::loadTextureFromCsxFile(levelTriggerPath, renderer, trigger, error)) {
            LogFmt("Loading texture for trigger failed. {}", *error);
            return {};
        }
        levelData.triggers.push_back(std::move(trigger));
    }

    return std::make_optional(std::move(levelObj));
}

std::string Level::levelWindowName(std::string_view levelName, LevelType levelType)
{
    if (levelType == LevelType::kSingle) {
        return std::string(levelName);
    } else {
        return std::format("{} [mp]", levelName);
    }
}

std::string Level::levelMainDir(std::string_view rootDirectory, std::string_view levelType, std::string_view levelName)
{
    return std::format("{}/levels/{}/{}", rootDirectory, levelType, levelName);
}

std::string Level::levelPackDir(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/pack/{}", rootDirectory, levelPack);
}

std::string Level::levelLvlDir(std::string_view rootDirectory)
{
    return std::format("{}/levels/lvl", rootDirectory);
}


std::string Level::levelSef(std::string_view rootDirectory, std::string_view levelType, std::string_view levelName)
{
    return std::format("{0}/levels/{1}/{2}/{2}.sef", rootDirectory, levelType, levelName);
}

std::string Level::levelSdb(std::string_view rootDirectory, std::string_view levelType, std::string_view levelName)
{
    return std::format("{0}/levels/{1}/{2}/{2}.sdb", rootDirectory, levelType, levelName);
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

std::string Level::levelTriggerDir(std::string_view rootDirectory, std::string_view levelPack)
{
    return std::format("{}/levels/pack/{}/bitmaps/triggers", rootDirectory, levelPack);
}

std::string Level::levelTrigger(std::string_view rootDirectory, std::string_view levelPack, int index)
{
    return std::format("{}/levels/pack/{}/bitmaps/triggers/trigger_{}.csx", rootDirectory, levelPack, index);
}
