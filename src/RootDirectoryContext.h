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

    const auto& singleLevelNames() const { return m_singleLevelNames; }
    const auto& multiplayerLevelNames() const { return m_multiplayerLevelNames; }

    const auto& csxFiles() const { return m_csxFiles; }
    const auto& sdbFiles() const { return m_sdbFiles; }
    const auto& mdfFiles() const { return m_mdfFiles; }
    const auto& csFiles() const { return m_csFiles; }

    const auto& levelHumanNamesDict() const { return m_levelHumanNamesDict; }
    const auto& dialogPhrases() const { return m_dialogPhrases; }
    const auto& globalVars() const { return m_globalVars; }

    std::vector<Level> levels;
    int selectedLevelIndex = 0;

    bool showCsxWindow = false;
    bool showSdbWindow = false;
    bool showMdfWindow = false;
    bool showCsWindow = false;

private:
    void asyncLoadResources(std::string_view rootDirectory);

    std::string m_rootDirectory;
    std::future<void> m_loadPathFuture;
    std::atomic<bool> m_isLoading{false};

    std::vector<std::string> m_singleLevelNames;
    std::vector<std::string> m_multiplayerLevelNames;

    std::vector<std::string> m_csxFiles;
    std::vector<std::string> m_sdbFiles;
    std::vector<std::string> m_mdfFiles;
    std::vector<std::string> m_csFiles;

    StringHashTable<std::string> m_levelHumanNamesDict;
    std::map<int, std::string> m_dialogPhrases;
    UMapStringVar_t m_globalVars;
};
