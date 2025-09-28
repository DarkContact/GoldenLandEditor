#include "Resources.h"

#include <filesystem>
#include <format>
#include <future>

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
    Magic_Bitmap
};

Resources::Resources(std::string_view rootDirectory) :
    m_rootDirectory(rootDirectory)
{
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
}

std::vector<std::string> Resources::levelNames() const
{
    std::vector<std::string> results;
    try {
        for (const auto& entry : fs::directory_iterator(std::format("{}/levels/single", m_rootDirectory))) {
            results.push_back(entry.path().filename().string());
        }
    } catch (const fs::filesystem_error& e) {
        fprintf(stderr, "Filesystem error: %s\n", e.what());
    }
    return results;
}

std::vector<std::string> Resources::sdbFiles() const
{
    return filesWithExtensionAsync({Levels_Single, Levels_Multiplayer, Sdb}, ".sdb");
}

std::vector<std::string> Resources::csxFiles() const
{
    return filesWithExtensionAsync({Engineres, Levels_Pack, Magic_Bitmap, Persons, Wear}, ".csx");
}

std::vector<std::string> Resources::Resources::filesWithExtension(std::initializer_list<int> indices, std::string_view extension) const {
    std::vector<std::string> files;
    for (int index : indices) {
        if (!fs::is_directory(m_mainDirectories[index])) continue;

        for (const auto& entry : fs::recursive_directory_iterator(m_mainDirectories[index])) {
            if (entry.path().extension() == extension) {
                files.push_back(entry.path().lexically_relative(m_rootDirectory).string());
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
            std::vector<std::string> localFiles;
            if (!fs::is_directory(m_mainDirectories[index])) return localFiles;

            for (const auto& entry : fs::recursive_directory_iterator(m_mainDirectories[index])) {
                if (entry.path().extension() == extension) {
                    localFiles.push_back(entry.path().lexically_relative(m_rootDirectory).string());
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
