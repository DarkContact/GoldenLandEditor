#include "FontSettings.h"

#include <vector>
#include <string>
#include <filesystem>

#include "imgui.h"
#include "ImguiHelper.h"

void FontSettings::update(bool& showWindow)
{
    ImGuiStyle& style = ImGui::GetStyle();
    static int fontSize = style.FontSizeBase;
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
    ImGuiIO& io = ImGui::GetIO();
    static ImFont* selectedFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);

    if (showWindow)
        ImGui::OpenPopup("Font Settings");

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.35f), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Font Settings", &showWindow, ImGuiWindowFlags_NoResize)) {

        if (!fontNames.empty()) {
            if (ImguiHelper::ComboBoxWithIndex("Font", fontNames, selectedFontIndex)) {
                selectedFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);
            }

            ImGui::SliderInt("Size", &fontSize, 8, 32, "%d", ImGuiSliderFlags_ClampOnInput);

            if (ImGui::Button("Apply")) {
                style.FontSizeBase = fontSize;
                style._NextFrameFontSizeBase = style.FontSizeBase;

                ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);
                if (newFont) {
                    io.FontDefault = newFont;
                } else {
                    printf("Failed to load font: %s", fontPaths[selectedFontIndex].c_str());
                }
            }

            if (selectedFont) {
                ImGui::Separator();

                ImGui::BeginChild("Font Preview", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);

                ImGui::PushFont(selectedFont, fontSize);
                ImGui::Text("The quick brown fox jumps over the lazy dog. 1234567890");
                ImGui::Text("Съешь же ещё этих мягких французских булок, да выпей чаю.");
                ImGui::PopFont();

                ImGui::EndChild();
            }
        }

        ImGui::EndPopup();
    }
}
