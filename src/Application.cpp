#include "Application.h"
#include "Settings.h"

#include <atomic>
#include <future>
#include <format>

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "Resources.h"
#include "embedded_resources.h"
#include "utils/ImGuiWidgets.h"
#include "windows/FontSettings.h"
#include "windows/LevelPicker.h"
#include "windows/LevelViewer.h"
#include "windows/CsxViewer.h"
#include "windows/SdbViewer.h"
#include "windows/MdfViewer.h"
#include "windows/CsViewer.h"

#include "utils/TracyProfiler.h"
#include "utils/DebugLog.h"

namespace {
    std::atomic_bool backgroundWork = false;
}

void Application::RootDirectoryContext::setRootDirectoryAndReload(std::string_view rootDirectory) {
    backgroundWork = true;

    levels.clear(); // TODO: Что-то делать с уровнями если остались несохранённые данные
    selectedLevelIndex = 0;

    showCsxWindow = false;
    showSdbWindow = false;
    showMdfWindow = false;
    showCsWindow = false;

    asyncLoadPaths(rootDirectory); // TODO: Запись rootDirectory в ini файл настроек
}

void Application::RootDirectoryContext::asyncLoadPaths(std::string_view rootDirectory) {
    backgroundWork = true;
    this->m_rootDirectory = rootDirectory;
    auto backgroundTask = [] (RootDirectoryContext* rootDirContext) {
        Resources resources(rootDirContext->rootDirectory());
        rootDirContext->singleLevelNames = resources.levelNames(LevelType::kSingle);
        rootDirContext->multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);
        rootDirContext->csxFiles = resources.csxFiles();
        rootDirContext->sdbFiles = resources.sdbFiles();
        rootDirContext->mdfFiles = resources.mdfFiles();
        rootDirContext->csFiles = resources.csFiles();
        backgroundWork = false;
    };
    m_loadPathFuture = std::async(std::launch::async, backgroundTask, this);
}

Application::Application() {
    Settings settings("settings.ini");
    std::string fontFilepath = settings.readString(Setting::kFontFilepath);
    int fontSize = settings.readInt(Setting::kFontSize, 13);
    std::string rootDir = settings.readString(Setting::kRootDir);
    if (!rootDir.empty()) {
        m_rootDirContext.setRootDirectoryAndReload(rootDir);
    }

    initSdl();
    initImGui(fontFilepath, fontSize);
}

Application::~Application() {
    shutdown();
}

void Application::initSdl() {
    LogFmt("[INFO] SDL Version: {}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    if (SDL_VERSION != SDL_GetVersion()) {
        LogFmt("[INFO] Linked SDL Version: {}.{}.{}",
               SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MICRO(SDL_GetVersion()));
    }

    // Линкуемая версия должна быть не ниже компилируемой
    if (SDL_VERSION > SDL_GetVersion()) {
        Log("[ERROR] Compiled SDL version less when linked SDL version!");
        throw std::runtime_error("Compiled SDL version less when linked SDL version!");
    }

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LogFmt("SDL_Init error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_Init error: {}", SDL_GetError()));
    }

    // Create window with SDL_Renderer graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    m_window = SDL_CreateWindow(std::format("Goldenland Editor v{}", GOLDENLAND_VERSION_STRING).c_str(),
                                          (int)(1024 * main_scale), (int)(768 * main_scale), window_flags);
    if (m_window == nullptr) {
        LogFmt("SDL_CreateWindow error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_CreateWindow error: {}", SDL_GetError()));
    }
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (m_renderer == nullptr) {
        LogFmt("SDL_CreateRenderer error: {}", SDL_GetError());
        throw std::runtime_error(std::format("SDL_CreateRenderer error: {}", SDL_GetError()));
    }
    SDL_SetRenderVSync(m_renderer, 1);

    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(m_window);
}

void Application::initImGui(std::string_view fontFilepath, int fontSize) {
    const char* imguiBranch =
#ifdef IMGUI_HAS_DOCK
        "docking";
#else
        "master";
#endif

    LogFmt("[INFO] ImGui Version: {} (branch: {})", IMGUI_VERSION, imguiBranch);
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    style.ScaleAllSizes(mainScale);
    style.FontScaleDpi = mainScale;

    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 3.0f;

    style.WindowTitleAlign.x = 0.5f;
    style.WindowTitleAlign.y = 0.75f;

    ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer3_Init(m_renderer);

    // Встраиваемый шрифт
    ImFontConfig embeddedFontConfig;
    embeddedFontConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)carlito_ttf, carlito_ttf_size, 0.0f, &embeddedFontConfig);

    // Загрузка шрифта и его размера
    if (!fontFilepath.empty()) {
        ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontFilepath.data(), fontSize);
        io.FontDefault = newFont;
    }

    style.FontSizeBase = fontSize;
    style._NextFrameFontSizeBase = style.FontSizeBase;
}

bool Application::hasActiveAnimations() const {
    if (backgroundWork) {
        return true;
    }
    if (m_mdfViewer.isAnimating()) {
        return true;
    }
    for (const auto& level : m_rootDirContext.levels) {
        if (m_levelViewer.isAnimating(level)) {
            return true;
        }
    }
    return false;
}

void Application::mainLoop() {
    std::string uiError;

    while (!m_done)
    {
        bool hasEvents = processEvents(m_renderCooldown > 0);

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        static bool showSettingsWindow = false;
        static bool showLevelsWindow = false;

        bool loaderWindow = backgroundWork.load();
        ImGuiWidgets::Loader("Loading...", loaderWindow);
        ImGuiWidgets::ShowMessageModal("Error", uiError);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Levels")) {
                    showLevelsWindow = !m_rootDirContext.singleLevelNames.empty();
                }
                if (ImGui::MenuItem("CSX Viewer")) {
                    m_rootDirContext.showCsxWindow = !m_rootDirContext.csxFiles.empty();
                }
                if (ImGui::MenuItem("SDB Viewer")) {
                    m_rootDirContext.showSdbWindow = !m_rootDirContext.sdbFiles.empty();
                }
                if (ImGui::MenuItem("MDF Viewer")) {
                    m_rootDirContext.showMdfWindow = !m_rootDirContext.mdfFiles.empty();
                }
                if (ImGui::MenuItem("CS Viewer")) {
                    m_rootDirContext.showCsWindow = !m_rootDirContext.csFiles.empty();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Root folder")) {
                    // TODO: Сделать модальное окно с отображением текущей директории
                    SDL_ShowOpenFolderDialog([] (void* userdata, const char* const* filelist, int filter) {
                        if (!filelist) {
                            LogFmt("Folder dialog error: {}", SDL_GetError());
                            return;
                        } else if (!*filelist) {
                            Log("Dialog was canceled");
                            return;
                        } else if ((*filelist)[0] == '\0') {
                            Log("Filelist empty");
                            return;
                        }

                        if (*filelist) {
                            LogFmt("Selected folder: {}", *filelist);
                            Application* app = static_cast<Application*>(userdata);
                            if (app->m_rootDirContext.rootDirectory() == *filelist) return;

                            app->m_rootDirContext.setRootDirectoryAndReload(*filelist);
                        }
                    }, this, m_window, m_rootDirContext.rootDirectory().data(), false);
                }
                if (ImGui::MenuItem("Fonts")) {
                    showSettingsWindow = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (showSettingsWindow) {
            FontSettings::update(showSettingsWindow);
        }
        if (!backgroundWork) {
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_csxViewer.update(m_rootDirContext.showCsxWindow, m_renderer, m_rootDirContext.rootDirectory(), m_rootDirContext.csxFiles);
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_sdbViewer.update(m_rootDirContext.showSdbWindow, m_rootDirContext.rootDirectory(), m_rootDirContext.sdbFiles);
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_mdfViewer.update(m_rootDirContext.showMdfWindow, m_renderer, m_rootDirContext.rootDirectory(), m_rootDirContext.mdfFiles);
            ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
            m_csViewer.update(m_rootDirContext.showCsWindow, m_rootDirContext.rootDirectory(), m_rootDirContext.csFiles);
        }

        // NOTE: Для генерации озвучки
        // static bool csViewerOnce = false;
        // if (!backgroundWork && !csViewerOnce) {
        //     m_csViewer.injectPlaySoundAndGeneratePhrases("C:/Games/Холодные Небеса", m_rootDirContext.rootDirectory(), m_rootDirContext.csFiles);
        //     csViewerOnce = true;
        // }

        if (showLevelsWindow && !m_rootDirContext.singleLevelNames.empty()) {
            if (auto result = m_levelPicker.update(showLevelsWindow,
                                                  m_rootDirContext.singleLevelNames,
                                                  m_rootDirContext.multiplayerLevelNames,
                                                  m_rootDirContext.selectedLevelIndex); result.selected) {
                bool alreadyLoaded = false;
                for (const auto& level : m_rootDirContext.levels) {
                    if (level.data().name == result.loadedLevelName && level.data().type == result.loadedLevelType) {
                        alreadyLoaded = true;
                        break;
                    }
                }

                if (alreadyLoaded) {
                    ImGui::SetWindowFocus(Level::levelWindowName(result.loadedLevelName, result.loadedLevelType).c_str());
                } else {
                    // Загрузка уровня
                    std::string error;
                    auto level = Level::loadLevel(m_renderer, m_rootDirContext.rootDirectory(), result.loadedLevelName, result.loadedLevelType, &error);
                    if (level) {
                        m_rootDirContext.levels.push_back(std::move(*level));
                    } else {
                        uiError = std::move(error);
                    }
                }
            }
        }

        if (!backgroundWork) {
            for (auto it = m_rootDirContext.levels.begin(); it != m_rootDirContext.levels.end();) {
                bool openLevel = true;
                Level& level = *it;
                if (level.data().background) {
                    ImGui::SetNextWindowDockID(mainDockSpace, ImGuiCond_FirstUseEver);
                    m_levelViewer.update(openLevel, m_rootDirContext.rootDirectory(), level);
                    if (!openLevel) {
                        it = m_rootDirContext.levels.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }
        
        if (hasActiveAnimations() || hasEvents) {
            m_renderCooldown = kRenderCooldownFrames;
        } else if (m_renderCooldown > 0) {
            m_renderCooldown--;
        }

        render();
    }
}

bool Application::processEvents(bool noWait) {
    SDL_Event event;
    bool hasEvent = noWait ? SDL_PollEvent(&event)
                           : SDL_WaitEventTimeout(&event, kWaitTimeoutMs);

    bool hasEvents = hasEvent;
    while (hasEvent) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT)
            m_done = true;

        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(m_window))
        {
            m_done = true;
        }

        hasEvent = SDL_PollEvent(&event);
    }
    return hasEvents;
}

void Application::render() {
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();

    {
        Tracy_ZoneScopedN("Rendering prepare");
        Tracy_ZoneColor(0xadff2f); // GreenYellow

        ImGui::Render();

        SDL_SetRenderScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(m_renderer, clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        SDL_RenderClear(m_renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    }

    {
        Tracy_ZoneScopedN("CaptureImage");
        Tracy_ZoneColor(0xc0c0c0); // Silver

        Tracy_CaptureImage(m_renderer);
    }

    {
        Tracy_ZoneScopedN("Rendering");
        Tracy_ZoneColor(0x32cd32); // LimeGreen

        SDL_RenderPresent(m_renderer);
    }

    Tracy_FrameMark;
}

void Application::shutdown() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
