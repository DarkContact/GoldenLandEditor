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

    const std::vector<std::string>& singleLevelNames() const { return m_singleLevelNames; }
    const std::vector<std::string>& multiplayerLevelNames() const { return m_multiplayerLevelNames; }
    const StringHashTable<std::string>& levelHumanNamesDict() const { return m_levelHumanNamesDict; }

    const std::vector<std::string>& csxFiles() const { return m_csxFiles; }
    const std::vector<std::string>& sdbFiles() const { return m_sdbFiles; }
    const std::vector<std::string>& mdfFiles() const { return m_mdfFiles; }
    const std::vector<std::string>& csFiles() const { return m_csFiles; }

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

    std::vector<std::string> m_singleLevelNames;
    std::vector<std::string> m_multiplayerLevelNames;
    StringHashTable<std::string> m_levelHumanNamesDict;

    std::vector<std::string> m_csxFiles;
    std::vector<std::string> m_sdbFiles;
    std::vector<std::string> m_mdfFiles;
    std::vector<std::string> m_csFiles;
};
