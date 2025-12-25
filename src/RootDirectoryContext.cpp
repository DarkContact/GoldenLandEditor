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
            singleLevelNames.empty() &&
            multiplayerLevelNames.empty() &&
            csxFiles.empty() &&
            sdbFiles.empty() &&
            mdfFiles.empty() &&
            csFiles.empty();

    return m_rootDirectory.empty() || emptyResources;
}

void RootDirectoryContext::asyncLoadPaths(std::string_view rootDirectory) {
    m_isLoading = true;
    this->m_rootDirectory = rootDirectory;
    auto backgroundTask = [] (RootDirectoryContext* context) {
        Resources resources(context->rootDirectory());
        context->singleLevelNames = resources.levelNames(LevelType::kSingle);
        context->multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);
        context->levelHumanNamesDict = resources.levelHumanNameDictionary();

        context->csxFiles = resources.csxFiles();
        context->sdbFiles = resources.sdbFiles();
        context->mdfFiles = resources.mdfFiles();
        context->csFiles = resources.csFiles();

        context->m_isLoading = false;
    };
    m_loadPathFuture = std::async(std::launch::async, backgroundTask, this);
}
