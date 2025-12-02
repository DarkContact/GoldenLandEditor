#pragma once
#include <vector>
#include <string>
#include <future>

#include "Level.h"
#include "windows/LevelPicker.h"
#include "windows/LevelViewer.h"
#include "windows/CsxViewer.h"
#include "windows/SdbViewer.h"
#include "windows/MdfViewer.h"
#include "windows/CsViewer.h"

struct SDL_Window;
struct SDL_Renderer;

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    void mainLoop();

private:
    void initSdl();
    void initImGui(std::string_view fontFilepath, int fontSize);

    bool processEvents(bool noWait);
    void render();

    void shutdown();

    bool hasActiveAnimations() const;

    struct RootDirectoryContext {
        std::vector<std::string> singleLevelNames;
        std::vector<std::string> multiplayerLevelNames;
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

        std::string_view rootDirectory() const { return m_rootDirectory; }
        void setRootDirectoryAndReload(std::string_view rootDirectory);

    private:
        void asyncLoadPaths(std::string_view rootDirectory);

        std::string m_rootDirectory;
        std::future<void> m_loadPathFuture;
    };

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    RootDirectoryContext m_rootDirContext;
    LevelPicker m_levelPicker;
    LevelViewer m_levelViewer;
    CsxViewer m_csxViewer;
    SdbViewer m_sdbViewer;
    MdfViewer m_mdfViewer;
    CsViewer m_csViewer;
    bool m_done = false;

    // Оптимизация обновления логики и отрисовки
    static const int kWaitTimeoutMs = 250;
    static const int kRenderCooldownFrames = 10;
    int m_renderCooldown = 0;
};
