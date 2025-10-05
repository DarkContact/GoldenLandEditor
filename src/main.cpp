#include <atomic>
#include <future>
#include <cstdio>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "ini.h"

#include "Level.h"
#include "Resources.h"
#include "utils/ImguiHelper.h"
#include "windows/FontSettings.h"
#include "windows/LevelPicker.h"
#include "windows/LevelViewer.h"
#include "windows/CsxViewer.h"
#include "windows/SdbViewer.h"

#include "utils/TracyProfiler.h"

// TODO: Сохранение уровня
// TODO: Изменение данных уровня с поддержкой undo/redo

struct RootDirectoryContext {
    std::vector<std::string> levelNames;
    std::vector<std::string> csxFiles;
    std::vector<std::string> sdbFiles;
    std::vector<Level> levels;
    int selectedLevelIndex = 0;
    bool showCsxWindow = false;
    bool showSdbWindow = false;

    const std::string& rootDirectory() const { return m_rootDirectory; }

    void setRootDirectory(const std::string& rootDirectory) {
        levels.clear(); // TODO: Что-то делать с уровнями если остались несохранённые данные
        selectedLevelIndex = 0;
        showCsxWindow = false;
        showSdbWindow = false;
        m_rootDirectory = rootDirectory;
    }

private:
    std::string m_rootDirectory;
};

std::atomic_bool backgroundWork = false;

int main(int, char**)
{
    RootDirectoryContext rootDirectoryContext;

    std::string fontFilepath;
    int fontSize = 13;

    auto settingsFilename = "settings.ini";
    mINI::INIFile settingsFile(settingsFilename);
    mINI::INIStructure settingsFileIni;
    if (std::filesystem::exists(settingsFilename)) {
        settingsFile.read(settingsFileIni);
        fontFilepath = settingsFileIni.get("fonts").get("filepath");
        auto fontSizeString = settingsFileIni.get("fonts").get("size");
        if (!fontSizeString.empty())
            fontSize = std::stoi(fontSizeString);

        rootDirectoryContext.setRootDirectory(settingsFileIni.get("resources").get("root_dir"));
    } else {
        settingsFileIni["fonts"]["filepath"] = fontFilepath;
        settingsFileIni["fonts"]["size"] = fontSize;
        settingsFile.write(settingsFileIni, true);
    }

    auto backgroundTask = [] (RootDirectoryContext& rootDirectoryContext) {
        backgroundWork = true;
        Resources resources(rootDirectoryContext.rootDirectory());
        rootDirectoryContext.levelNames = resources.levelNames();
        rootDirectoryContext.csxFiles = resources.csxFiles();
        rootDirectoryContext.sdbFiles = resources.sdbFiles();
        backgroundWork = false;
    };
    auto bgTaskFuture = std::async(std::launch::async, backgroundTask, std::ref(rootDirectoryContext));

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Create window with SDL_Renderer graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("Goldenland Editor", (int)(1024 * main_scale), (int)(768 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

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
        ImguiHelper::Loader("Loading...", loaderWindow);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Levels")) {
                    showLevelsWindow = !rootDirectoryContext.levelNames.empty();
                }
                if (ImGui::MenuItem("CSX Viewer")) {
                    rootDirectoryContext.showCsxWindow = !rootDirectoryContext.csxFiles.empty();
                }
                if (ImGui::MenuItem("SDB Viewer")) {
                    rootDirectoryContext.showSdbWindow = !rootDirectoryContext.sdbFiles.empty();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Root folder")) {
                    // TODO: Сделать модальное окно с отображением текущей директории
                    SDL_ShowOpenFolderDialog([] (void* userdata, const char* const* filelist, int filter) {
                        if (!filelist) {
                            SDL_Log("Folder dialog error: %s", SDL_GetError());
                            return;
                        } else if (!*filelist) {
                            // Dialog was canceled.
                            return;
                        } else if ((*filelist)[0] == '\0') {
                            // Выбрана всякая чушь из панели управления
                            return;
                        }

                        if (*filelist) {
                            RootDirectoryContext* rootDirectoryContext = static_cast<RootDirectoryContext*>(userdata);
                            if (rootDirectoryContext->rootDirectory() == *filelist) return;

                            backgroundWork = true;
                            rootDirectoryContext->setRootDirectory(*filelist);
                            auto backgroundTask = [] (RootDirectoryContext* rootDirectoryContext) {
                              Resources resources(rootDirectoryContext->rootDirectory());
                                rootDirectoryContext->levelNames = resources.levelNames();
                                rootDirectoryContext->csxFiles = resources.csxFiles();
                                rootDirectoryContext->sdbFiles = resources.sdbFiles();
                                // TODO: Запись rootDirectory в ini файл настроек
                                backgroundWork = false;
                            };
                            auto bgTaskFuture = std::async(std::launch::async, backgroundTask, rootDirectoryContext);
                        }
                    }, &rootDirectoryContext, window, rootDirectoryContext.rootDirectory().c_str(), false);
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

        std::string loadedLevelName;
        if (showLevelsWindow && !rootDirectoryContext.levelNames.empty()) {
            if (LevelPicker::update(showLevelsWindow, rootDirectoryContext.levelNames, rootDirectoryContext.selectedLevelIndex)) {
                loadedLevelName = rootDirectoryContext.levelNames[rootDirectoryContext.selectedLevelIndex];
                bool alreadyLoaded = false;
                for (const auto& level : rootDirectoryContext.levels) {
                    if (level.data().name == loadedLevelName) {
                        alreadyLoaded = true;
                        break;
                    }
                }

                if (alreadyLoaded) {
                    ImGui::SetWindowFocus(loadedLevelName.c_str());
                } else {
                    // Загрузка уровня
                    rootDirectoryContext.levels.emplace_back(renderer, rootDirectoryContext.rootDirectory(), loadedLevelName);
                }
            }
        }

        for (auto it = rootDirectoryContext.levels.begin(); it != rootDirectoryContext.levels.end();) {
            bool openLevel = true;
            Level& level = *it;
            if (level.data().background) {
                LevelViewer::update(openLevel, level);

                if (!openLevel) {
                    it = rootDirectoryContext.levels.erase(it);
                    continue;
                }

                ++it;
            }
        }

        // Rendering
        {
            Tracy_ZoneScopedN("Rendering");
            Tracy_ZoneColor(0x32cd32); // LimeGreen

            ImGui::Render();

            SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            SDL_RenderClear(renderer);

            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        }

        {
            Tracy_ZoneScopedN("CaptureImage");
            Tracy_ZoneColor(0xc0c0c0); // Silver

            CaptureImage(renderer);
        }

        SDL_RenderPresent(renderer);

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
