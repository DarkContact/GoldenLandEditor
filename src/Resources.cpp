#include "Resources.h"

#include <filesystem>
#include <format>
#include <future>

#include "parsers/SDB_Parser.h"
#include "CsExecutor.h"

#include "utils/DebugLog.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

namespace fs = std::filesystem;

enum MainDirectories {
    Engineres = 0,
    Items,
    Levels,
    Magic,
    Music,
    Persons,
    Scripts,
    Sdb,
    Sounds,
    Wear,

    Levels_Pack,
    Levels_Single,
    Levels_Multiplayer,
    Magic_Bitmap,
    Scripts_Dialogs
};

Resources::Resources(std::string_view rootDirectory) :
    m_rootDirectory(rootDirectory)
{
    Tracy_ZoneScoped;
    m_mainDirectories[Engineres] = std::format("{}/engineres", m_rootDirectory);
    m_mainDirectories[Items] = std::format("{}/items", m_rootDirectory);
    m_mainDirectories[Levels] = std::format("{}/levels", m_rootDirectory);
    m_mainDirectories[Magic] = std::format("{}/magic", m_rootDirectory);
    m_mainDirectories[Music] = std::format("{}/music", m_rootDirectory);
    m_mainDirectories[Persons] = std::format("{}/persons", m_rootDirectory);
    m_mainDirectories[Scripts] = std::format("{}/scripts", m_rootDirectory);
    m_mainDirectories[Sdb] = std::format("{}/sdb", m_rootDirectory);
    m_mainDirectories[Sounds] = std::format("{}/sounds", m_rootDirectory);
    m_mainDirectories[Wear] = std::format("{}/wear", m_rootDirectory);

    m_mainDirectories[Levels_Pack] = std::format("{}/levels/pack", m_rootDirectory);
    m_mainDirectories[Levels_Single] = std::format("{}/levels/single", m_rootDirectory);
    m_mainDirectories[Levels_Multiplayer] = std::format("{}/levels/multiplayer", m_rootDirectory);
    m_mainDirectories[Magic_Bitmap] = std::format("{}/magic/bitmap", m_rootDirectory);
    m_mainDirectories[Scripts_Dialogs] = std::format("{}/scripts/dialogs", m_rootDirectory);
}

std::vector<std::string> Resources::levelNames(LevelType type) const
{
    Tracy_ZoneScoped;
    std::vector<std::string> results;
    try {
        auto dir = std::format("{}/levels/{}", m_rootDirectory, levelTypeToString(type));
        if (!fs::exists(StringUtils::toUtf8View(dir)))
            return results;

        for (const auto& entry : fs::directory_iterator(StringUtils::toUtf8View(dir))) {
            results.push_back(entry.path().filename().string());
        }
    } catch (const fs::filesystem_error& ex) {
        LogFmt("Filesystem error: {}", ex.what());
    }
    return results;
}

std::vector<std::string> Resources::sdbFiles() const
{
    return filesWithExtension({Levels_Single, Levels_Multiplayer, Sdb}, ".sdb");
}

std::vector<std::string> Resources::csxFiles() const
{
    return filesWithExtensionAsync({Engineres, Levels_Pack, Magic_Bitmap, Persons, Wear}, ".csx");
}

std::vector<std::string> Resources::mdfFiles() const
{
    return filesWithExtension({Magic}, ".mdf"); // TODO: fs::directory_iterator
}

std::vector<std::string> Resources::csFiles() const
{
    return filesWithExtension({Scripts_Dialogs}, ".cs");
}

StringHashTable<std::string> Resources::levelHumanNameDictionary() const
{
    std::string error;

    SDB_Data techNames;
    if (!SDB_Parser::parse(std::format("{}/globalmap/gm_loc_tech.sdb", m_mainDirectories[Sdb]), techNames, &error))
        LogFmt("gm_loc_tech.sdb error: {}", error);

    SDB_Data visNames;
    if (!SDB_Parser::parse(std::format("{}/globalmap/gm_reg_vis.sdb", m_mainDirectories[Sdb]), visNames, &error))
        LogFmt("gm_reg_vis.sdb error: {}", error);

    SDB_Data litNames;
    if (!SDB_Parser::parse(std::format("{}/globalmap/gm_loc_lit.sdb", m_mainDirectories[Sdb]), litNames, &error))
        LogFmt("gm_loc_lit.sdb error: {}", error);

    StringHashTable<std::string> result;
    for (const auto& [key, value] : techNames.strings) {
        auto techName = StringUtils::trimRight(value);
        if (techName.empty())
            continue;

        // Если есть кириллица
        if (static_cast<unsigned char>(techName[0]) >= 128)
            continue;

        std::string_view visName = visNames.strings[key];
        std::string_view litName = litNames.strings[key];
        if (visName == "Item inserting") {
            visName = {};
        }
        if (litName == "Item inserting") {
            litName = {};
        }


        std::string humanName;
        if (litName.empty()) {
            humanName = std::format("{}", visName);
        } else if (visName.empty()) {
            humanName = std::format("{}", litName);
        } else {
            humanName = std::format("{} ({})", visName, litName);
        }

        if (humanName.empty())
            continue;

        std::string techNameLower(techName);
        StringUtils::toLowerAscii(techNameLower, techNameLower);
        result.emplace(techNameLower, humanName);
    }

    return result;
}

std::map<int, std::string> Resources::dialogPhrases() const
{
    std::string error;
    SDB_Data sdbDialogs;
    std::string sdbPath = std::format("{}/dialogs/dialogsphrases.sdb", m_mainDirectories[Sdb]);
    if (!SDB_Parser::parse(sdbPath, sdbDialogs, &error))
        LogFmt("Load dialogsphrases.sdb error: {}", error);

    return sdbDialogs.strings;
}

UMapStringVar_t Resources::globalVars() const
{
    std::string error;
    std::string varsPath = std::format("{}/dialogs_special/zlato_vars.scr", m_mainDirectories[Scripts]);
    UMapStringVar_t globalVars;
    if (!CsExecutor::readGlobalVariables(varsPath, globalVars, &error))
        LogFmt("Load zlato_vars.scr error: {}", error);

    return globalVars;
}

std::vector<std::string> Resources::Resources::filesWithExtension(std::initializer_list<int> indices, std::string_view extension) const {
    std::vector<std::string> files;
    for (int index : indices) {
        auto dir = StringUtils::toUtf8View(m_mainDirectories[index]);
        if (!fs::is_directory(dir)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(dir)) { // TODO: Добавить версию с fs::directory_iterator
            if (entry.path().extension() == extension) {
                files.push_back(entry.path().lexically_relative(StringUtils::toUtf8View(m_rootDirectory)).string());
            }
        }
    }
    return files;
}

std::vector<std::string> Resources::filesWithExtensionAsync(std::initializer_list<int> indices, std::string_view extension) const {
    std::vector<std::future<std::vector<std::string>>> futures;
    futures.reserve(indices.size());

    // Запускаем асинхронные задачи для каждой директории
    for (int index : indices) {
        futures.push_back(std::async(std::launch::async, [this, index, extension]() {
            auto dir = StringUtils::toUtf8View(m_mainDirectories[index]);
            std::vector<std::string> localFiles;
            if (!fs::is_directory(dir)) return localFiles;

            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.path().extension() == extension) {
                    localFiles.push_back(entry.path().lexically_relative(StringUtils::toUtf8View(m_rootDirectory)).string());
                }
            }
            return localFiles;
        }));
    }

    // Собираем результаты из всех потоков
    std::vector<std::string> allFiles;
    for (auto& future : futures) {
        auto localFiles = future.get();
        allFiles.insert(allFiles.end(), localFiles.begin(), localFiles.end());
    }

    return allFiles;
}
