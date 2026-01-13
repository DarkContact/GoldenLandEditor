#include "RootDirectoryContext.h"

#include "Resources.h"

#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

void RootDirectoryContext::setRootDirectoryAndReload(std::string_view rootDirectory) {
    m_isLoading = true;

    levels.clear(); // TODO: Что-то делать с уровнями если остались несохранённые данные
    selectedLevelIndex = 0;

    showCsxWindow = false;
    showSdbWindow = false;
    showMdfWindow = false;
    showCsWindow = false;

    asyncLoadResources(rootDirectory); // TODO: Запись rootDirectory в ini файл настроек
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

void RootDirectoryContext::asyncLoadResources(std::string_view rootDirectory) {
    m_isLoading = true;
    this->m_rootDirectory = rootDirectory;
    auto backgroundTask = [] (RootDirectoryContext* context) {
        Resources resources(context->rootDirectory());
        {
            Tracy_ZoneScopedN("ReadFiles");
            context->m_singleLevelNames = resources.levelNames(LevelType::kSingle);
            context->m_multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);

            context->m_csxFiles = resources.csxFiles();
            context->m_sdbFiles = resources.sdbFiles();
            context->m_mdfFiles = resources.mdfFiles();
            context->m_csFiles = resources.csFiles();
        }
        {
            Tracy_ZoneScopedN("NaturalSort");
            std::sort(context->m_singleLevelNames.begin(), context->m_singleLevelNames.end(), StringUtils::naturalCompare);
            std::sort(context->m_multiplayerLevelNames.begin(), context->m_multiplayerLevelNames.end(), StringUtils::naturalCompare);

            std::sort(context->m_csxFiles.begin(), context->m_csxFiles.end(), StringUtils::naturalCompare);
            std::sort(context->m_sdbFiles.begin(), context->m_sdbFiles.end(), StringUtils::naturalCompare);
            std::sort(context->m_mdfFiles.begin(), context->m_mdfFiles.end(), StringUtils::naturalCompare);
            std::sort(context->m_csFiles.begin(), context->m_csFiles.end(), StringUtils::naturalCompare);
        }

        context->m_levelHumanNamesDict = resources.levelHumanNameDictionary();
        context->m_dialogPhrases = resources.dialogPhrases();
        context->m_globalVars = resources.globalVars();

        context->m_isLoading = false;
    };
    m_loadPathFuture = std::async(std::launch::async, backgroundTask, this);
}
