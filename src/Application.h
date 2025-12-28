#pragma once
#include <string_view>
#include <optional>

#include "RootDirectoryContext.h"
#include "windows/FontSettings.h"
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

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    RootDirectoryContext m_rootDirContext;

    std::optional<FontSettings> m_fontSettings;
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
