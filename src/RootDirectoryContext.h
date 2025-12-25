#pragma once
#include <vector>
#include <string>
#include <future>
#include <atomic>

#include "Level.h"
#include "Types.h"

class RootDirectoryContext {
public:
    void setRootDirectoryAndReload(std::string_view rootDirectory);
    bool isEmptyContext() const;
    
    std::string_view rootDirectory() const { return m_rootDirectory; }
    bool isLoading() const { return m_isLoading; }

    std::vector<std::string> singleLevelNames;
    std::vector<std::string> multiplayerLevelNames;
    StringHashTable<std::string> levelHumanNamesDict;

    std::vector<std::string> csxFiles;
    std::vector<std::string> sdbFiles;
    std::vector<std::string> mdfFiles;
    std::vector<std::string> csFiles;

    std::vector<Level> levels;
    int selectedLevelIndex = 0;

    bool showCsxWindow = false;
    bool showSdbWindow = false;
    bool showMdfWindow = false;
    bool showCsWindow = false;

private:
    void asyncLoadPaths(std::string_view rootDirectory);

    std::string m_rootDirectory;
    std::future<void> m_loadPathFuture;
    std::atomic<bool> m_isLoading{false};
};
