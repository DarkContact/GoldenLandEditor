#include <vector>
#include <string>
#include <filesystem>

#include <stdio.h>

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

int main(int, char**)
{
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

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
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

        // ImGui::ShowDemoWindow();

        static bool show_settings_window = false;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Menu")) {
                if (ImGui::MenuItem("Settings")) {
                    show_settings_window = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (show_settings_window) {
            ImGuiStyle& style = ImGui::GetStyle();
            static float fontSize = style.FontSizeBase;
            static int selectedFontIndex = 0;
            static std::vector<std::string> fontNames;
            static std::vector<std::string> fontPaths;
            static bool fontsLoaded = false;

            // Загружаем шрифты из системной папки Windows
            if (!fontsLoaded) {
                for (const auto& entry : std::filesystem::directory_iterator("C:/Windows/Fonts")) {
                    if (entry.path().extension() == ".ttf") {
                        fontPaths.push_back(entry.path().string());
                        fontNames.push_back(entry.path().stem().string());
                    }
                }
                fontsLoaded = true;
            }
            static ImFont* selectedFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);

            ImGui::Begin("Font Settings", &show_settings_window);

            if (!fontNames.empty()) {
                const char* currentFont = fontNames[selectedFontIndex].c_str();
                if (ImGui::BeginCombo("Font", currentFont)) {
                    for (int i = 0; i < fontNames.size(); ++i) {
                        bool isSelected = (i == selectedFontIndex);
                        if (ImGui::Selectable(fontNames[i].c_str(), isSelected)) {
                            selectedFontIndex = i;
                            selectedFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SliderFloat("Size", &fontSize, 8.0f, 48.0f, "%.0f");

                if (selectedFont) {
                    ImGui::PushFont(selectedFont, fontSize);
                    ImGui::Text("The quick brown fox jumps over the lazy dog. 1234567890");
                    ImGui::Text("Съешь же ещё этих мягких французских булок, да выпей чаю.");
                    ImGui::PopFont();
                }

                if (ImGui::Button("Apply")) {
                    style.FontSizeBase = fontSize;
                    style._NextFrameFontSizeBase = style.FontSizeBase;

                    ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);
                    if (newFont) {
                        io.FontDefault = newFont;
                    } else {
                        SDL_Log("Failed to load font: %s", fontPaths[selectedFontIndex].c_str());
                    }
                }
            }

            ImGui::End();
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
