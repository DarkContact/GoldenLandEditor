#include "FontSettings.h"

#include <vector>
#include <string>
#include <filesystem>

#include "imgui.h"

void FontSettings::update(bool& showWindow)
{
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
    ImGuiIO& io = ImGui::GetIO();
    static ImFont* selectedFont = io.Fonts->AddFontFromFileTTF(fontPaths[selectedFontIndex].c_str(), fontSize);

    ImGui::Begin("Font Settings", &showWindow);

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
                printf("Failed to load font: %s", fontPaths[selectedFontIndex].c_str());
            }
        }
    }

    ImGui::End();
}
