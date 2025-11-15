#include <atomic>
#include <future>
#include <format>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "Level.h"
#include "Resources.h"
#include "Settings.h"
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

// TODO: Сохранение уровня
// TODO: Изменение данных уровня с поддержкой undo/redo

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

    void setRootDirectory(std::string_view rootDirectory) {
        levels.clear(); // TODO: Что-то делать с уровнями если остались несохранённые данные
        selectedLevelIndex = 0;
        showCsxWindow = false;
        showSdbWindow = false;
        showMdfWindow = false;
        showCsWindow = false;
        m_rootDirectory = rootDirectory;
    }

private:
    std::string m_rootDirectory;
};

std::atomic_bool backgroundWork = false;

int main(int, char**)
{
    RootDirectoryContext rootDirectoryContext;

    Settings settings("settings.ini");
    std::string fontFilepath = settings.readString(Setting::kFontFilepath);
    int fontSize = settings.readInt(Setting::kFontSize, 13);
    std::string rootDir = settings.readString(Setting::kRootDir);
    if (!rootDir.empty()) {
        rootDirectoryContext.setRootDirectory(rootDir);
    }

    auto backgroundTask = [] (RootDirectoryContext& rootDirectoryContext) {
        backgroundWork = true;
        Resources resources(rootDirectoryContext.rootDirectory());
        rootDirectoryContext.singleLevelNames = resources.levelNames(LevelType::kSingle);
        rootDirectoryContext.multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);
        rootDirectoryContext.csxFiles = resources.csxFiles();
        rootDirectoryContext.sdbFiles = resources.sdbFiles();
        rootDirectoryContext.mdfFiles = resources.mdfFiles();
        rootDirectoryContext.csFiles = resources.csFiles();
        backgroundWork = false;
    };
    auto bgTaskFuture = std::async(std::launch::async, backgroundTask, std::ref(rootDirectoryContext));

    LogFmt("[INFO] SDL Version: {}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    if (SDL_VERSION != SDL_GetVersion()) {
        LogFmt("[INFO] Linked SDL Version: {}.{}.{}",
               SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
               SDL_VERSIONNUM_MICRO(SDL_GetVersion()));
    }

    // Линкуемая версия должна быть не ниже компилируемой
    if (SDL_VERSION > SDL_GetVersion()) {
        Log("Error. Compiled SDL version less when linked SDL version!");
        return -1;
    }

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LogFmt("SDL_Init error: {}", SDL_GetError());
        return -1;
    }

    // Create window with SDL_Renderer graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(std::format("Goldenland Editor v{}", GOLDENLAND_VERSION_STRING).c_str(),
                                          (int)(1024 * main_scale), (int)(768 * main_scale), window_flags);
    if (window == nullptr) {
        LogFmt("SDL_CreateWindow error: {}", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        LogFmt("SDL_CreateRenderer error: {}", SDL_GetError());
        return -1;
    }
    SDL_SetRenderVSync(renderer, 1);

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

#ifdef IMGUI_HAS_DOCK
    const char* imguiBranch = "docking";
#else
    const char* imguiBranch = "master";
#endif

    LogFmt("[INFO] ImGui Version: {} (branch: {})", IMGUI_VERSION, imguiBranch);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // style.FrameRounding = 3.0f;
    // style.WindowRounding = 5.0f;
    // style.GrabRounding = 2.0f;

    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 3.0f;

    style.WindowTitleAlign.x = 0.5f;
    style.WindowTitleAlign.y = 0.75f;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Применение настроек из ini файла
    if (!fontFilepath.empty()) {
        ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontFilepath.c_str(), fontSize);
        io.FontDefault = newFont;
    }
    style.FontSizeBase = fontSize;
    style._NextFrameFontSizeBase = style.FontSizeBase;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    std::string uiError;

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        //ImGui::ShowDemoWindow();

        static bool showSettingsWindow = false;
        static bool showLevelsWindow = false;

        static bool loaderWindow = false;
        loaderWindow = backgroundWork;
        ImGuiWidgets::Loader("Loading...", loaderWindow);
        ImGuiWidgets::ShowMessageModal("Error", uiError);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Levels")) {
                    showLevelsWindow = !rootDirectoryContext.singleLevelNames.empty();
                }
                if (ImGui::MenuItem("CSX Viewer")) {
                    rootDirectoryContext.showCsxWindow = !rootDirectoryContext.csxFiles.empty();
                }
                if (ImGui::MenuItem("SDB Viewer")) {
                    rootDirectoryContext.showSdbWindow = !rootDirectoryContext.sdbFiles.empty();
                }
                if (ImGui::MenuItem("MDF Viewer")) {
                    rootDirectoryContext.showMdfWindow = !rootDirectoryContext.mdfFiles.empty();
                }
                if (ImGui::MenuItem("CS Viewer")) {
                    rootDirectoryContext.showCsWindow = !rootDirectoryContext.csFiles.empty();
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
                            // Dialog was canceled.
                            return;
                        } else if ((*filelist)[0] == '\0') {
                            // Выбрана всякая чушь из панели управления
                            Log("Filelist empty");
                            return;
                        }

                        if (*filelist) {
                            LogFmt("Selected folder: {}", *filelist);
                            RootDirectoryContext* rootDirectoryContext = static_cast<RootDirectoryContext*>(userdata);
                            if (rootDirectoryContext->rootDirectory() == *filelist) return;

                            backgroundWork = true;
                            rootDirectoryContext->setRootDirectory(*filelist);
                            auto backgroundTask = [] (RootDirectoryContext* rootDirectoryContext) {
                                Resources resources(rootDirectoryContext->rootDirectory());
                                rootDirectoryContext->singleLevelNames = resources.levelNames(LevelType::kSingle);
                                rootDirectoryContext->multiplayerLevelNames = resources.levelNames(LevelType::kMultiplayer);
                                rootDirectoryContext->csxFiles = resources.csxFiles();
                                rootDirectoryContext->sdbFiles = resources.sdbFiles();
                                rootDirectoryContext->mdfFiles = resources.mdfFiles();
                                rootDirectoryContext->csFiles = resources.csFiles();
                                // TODO: Запись rootDirectory в ini файл настроек
                                backgroundWork = false;
                            };
                            auto bgTaskFuture = std::async(std::launch::async, backgroundTask, rootDirectoryContext);
                        }
                    }, &rootDirectoryContext, window, rootDirectoryContext.rootDirectory().data(), false);
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
        if (rootDirectoryContext.showCsxWindow && !rootDirectoryContext.csxFiles.empty()) {
            CsxViewer::update(rootDirectoryContext.showCsxWindow, renderer, rootDirectoryContext.rootDirectory(), rootDirectoryContext.csxFiles);
        }
        if (rootDirectoryContext.showSdbWindow && !rootDirectoryContext.sdbFiles.empty()) {
            SdbViewer::update(rootDirectoryContext.showSdbWindow, rootDirectoryContext.rootDirectory(), rootDirectoryContext.sdbFiles);
        }
        if (rootDirectoryContext.showMdfWindow && !rootDirectoryContext.mdfFiles.empty()) {
            MdfViewer::update(rootDirectoryContext.showMdfWindow, renderer, rootDirectoryContext.rootDirectory(), rootDirectoryContext.mdfFiles);
        }
        if (rootDirectoryContext.showCsWindow && !rootDirectoryContext.csFiles.empty()) {
            CsViewer::update(rootDirectoryContext.showCsWindow, rootDirectoryContext.rootDirectory(), rootDirectoryContext.csFiles);
        }

        if (showLevelsWindow && !rootDirectoryContext.singleLevelNames.empty()) {
            if (auto result = LevelPicker::update(showLevelsWindow,
                                                  rootDirectoryContext.singleLevelNames,
                                                  rootDirectoryContext.multiplayerLevelNames,
                                                  rootDirectoryContext.selectedLevelIndex); result.selected) {
                bool alreadyLoaded = false;
                for (const auto& level : rootDirectoryContext.levels) {
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
                    auto level = Level::loadLevel(renderer, rootDirectoryContext.rootDirectory(), result.loadedLevelName, result.loadedLevelType, &error);
                    if (level) {
                        rootDirectoryContext.levels.push_back(std::move(*level));
                    } else {
                        uiError = std::move(error);
                    }
                }
            }
        }

        if (!backgroundWork) {
            for (auto it = rootDirectoryContext.levels.begin(); it != rootDirectoryContext.levels.end();) {
                bool openLevel = true;
                Level& level = *it;
                if (level.data().background) {
                    LevelViewer::update(openLevel, rootDirectoryContext.rootDirectory(), level);

                    if (!openLevel) {
                        it = rootDirectoryContext.levels.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        }

        {
            Tracy_ZoneScopedN("Rendering prepare");
            Tracy_ZoneColor(0xadff2f); // GreenYellow

            ImGui::Render();

            SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            SDL_RenderClear(renderer);

            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        }

        {
            Tracy_ZoneScopedN("CaptureImage");
            Tracy_ZoneColor(0xc0c0c0); // Silver

            Tracy_CaptureImage(renderer);
        }

        {
            Tracy_ZoneScopedN("Rendering");
            Tracy_ZoneColor(0x32cd32); // LimeGreen

            SDL_RenderPresent(renderer);
        }

        Tracy_FrameMark;
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
