#include <stdio.h>

#include <SDL3/SDL.h>
#include <format>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "ini.h"

#include "Level.h"
#include "Resources.h"
#include "windows/FontSettings.h"
#include "windows/LevelPicker.h"
#include "windows/CsxViewer.h"

int main(int, char**)
{
    std::string fontFilepath;
    int fontSize = 13;

    std::string resourcesRootDirectory;

    auto settingsFilename = "settings.ini";
    mINI::INIFile settingsFile(settingsFilename);
    mINI::INIStructure settingsFileIni;
    if (std::filesystem::exists(settingsFilename)) {
        settingsFile.read(settingsFileIni);
        fontFilepath = settingsFileIni.get("fonts").get("filepath");
        auto fontSizeString = settingsFileIni.get("fonts").get("size");
        if (!fontSizeString.empty())
            fontSize = std::stoi(fontSizeString);

        resourcesRootDirectory = settingsFileIni.get("resources").get("root_dir");
    } else {
        settingsFileIni["fonts"]["filepath"] = fontFilepath;
        settingsFileIni["fonts"]["size"] = fontSize;
        settingsFile.write(settingsFileIni, true);
    }

    Resources resources(resourcesRootDirectory);
    auto levelNames = resources.levelNames();
    auto csxFiles = resources.csxFiles();

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
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
    std::vector<Level> levels;

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

        static bool show_settings_window = false;
        static bool show_levels_window = false;
        static bool show_csx_window = false;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Levels")) {
                    show_levels_window = true;
                }
                if (ImGui::MenuItem("CSX Viewer")) {
                    show_csx_window = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Fonts")) {
                    show_settings_window = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (show_settings_window) {
            FontSettings::update(show_settings_window);
        }

        if (show_csx_window) {
            CsxViewer::update(show_csx_window, renderer, resourcesRootDirectory, csxFiles);
        }

        std::string loadedLevelName;
        static int selectedLevelIndex = 0;
        if (show_levels_window) {
            if (LevelPicker::update(show_levels_window, levelNames, selectedLevelIndex)) {
                loadedLevelName = levelNames[selectedLevelIndex];
                bool alreadyLoaded = false;
                for (const auto& level : levels) {
                    if (level.data().name == loadedLevelName) {
                        alreadyLoaded = true;
                        break;
                    }
                }

                if (alreadyLoaded) {
                    ImGui::SetWindowFocus(loadedLevelName.c_str());
                } else {
                    levels.emplace_back(renderer, resourcesRootDirectory, loadedLevelName);
                }
            }
        }

        for (auto it = levels.begin(); it != levels.end();) {
            bool openLevel = true;
            Level& level = *it;
            if (level.data().background) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                ImGui::Begin(level.data().name.c_str(), &openLevel, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

                ImGui::PopStyleVar();

                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("View")) {
                        if (ImGui::MenuItem("Minimap", NULL, level.data().imgui.showMinimap)) {
                            level.data().imgui.showMinimap = !level.data().imgui.showMinimap;
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }

                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::Image((ImTextureID)level.data().background, ImVec2((float)level.data().background->w, (float)level.data().background->h));

                // ImGui::SetCursorScreenPos(pos);
                // ImGui::Text("%dx%d", level.data().background->w, level.data().background->h);

                if (level.data().minimap && level.data().imgui.showMinimap)
                {
                    float menu_bar_height = ImGui::GetCurrentWindow()->MenuBarHeight;
                    float scrollbar_width = (ImGui::GetCurrentWindow()->ScrollbarY ? style.ScrollbarSize : 0.0f);

                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    ImVec2 imageSize = ImVec2(level.data().minimap->w, level.data().minimap->h);

                    ImVec2 imagePos = ImVec2(windowPos.x + windowSize.x - imageSize.x - scrollbar_width - 8.0f,
                                             windowPos.y + menu_bar_height + 26.0f);
                    ImGui::SetCursorScreenPos(imagePos);

                    ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

                    ImGui::Image((ImTextureID)level.data().minimap, imageSize);

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                }

                ImGui::End();

                if (level.data().name == loadedLevelName) {
                    // Можно задокать только первое окно
                    ImGui::DockBuilderDockWindow(levels.front().data().name.c_str(), mainDockSpace);
                }

                if (!openLevel) {
                    it = levels.erase(it);
                    continue;
                }

                ++it;
            }
        }

        // Rendering
        ImGui::Render();
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
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
