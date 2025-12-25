#include "RootDirectoryContext.h"

#include "Resources.h"

void RootDirectoryContext::setRootDirectoryAndReload(std::string_view rootDirectory) {
    m_isLoading = true;

    levels.clear(); // TODO: Что-то делать с уровнями если остались несохранённые данные
    selectedLevelIndex = 0;

    showCsxWindow = false;
    showSdbWindow = false;
    showMdfWindow = false;
    showCsWindow = false;

    asyncLoadPaths(rootDirectory); // TODO: Запись rootDirectory в ini файл настроек
}

bool RootDirectoryContext::isEmptyContext() const {
    bool emptyResources =
            m_singleLevelNames.empty() &&
            m_multiplayerLevelNames.empty() &&
            m_csxFiles.empty() &&
            m_sdbFiles.empty() &&
            m_mdfFiles.empty() &&
            m_csFiles.empty();

    return m_rootDirectory.empty() || emptyResources;
}

void RootDirectoryContext::asyncLoadPaths(std::string_view rootDirectory) {
    m_isLoading = true;
    this->m_rootDirectory = rootDirectory;
    auto backgroundTask = [] (RootDirectoryContext* context) {
        Resources resources(context->rootDirectory());
        context->m_singleLevelNames = resources.levelNames(LevelType::kSingle);
        context->m_multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);
        context->m_levelHumanNamesDict = resources.levelHumanNameDictionary();

        context->m_csxFiles = resources.csxFiles();
        context->m_sdbFiles = resources.sdbFiles();
        context->m_mdfFiles = resources.mdfFiles();
        context->m_csFiles = resources.csFiles();

        context->m_isLoading = false;
    };
    m_loadPathFuture = std::async(std::launch::async, backgroundTask, this);
}
