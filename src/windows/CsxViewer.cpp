#include "CsxViewer.h"

#include <array>
#include <format>

#include "utils/TextureLoader.h"
#include "imgui.h"

#include "utils/TracyProfiler.h"

bool CsxViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles)
{
    Tracy_ZoneScopedN("CsxViewer::update");
    if (csxFiles.empty()) return false;

    static int selectedIndex = -1;
    static SDL_Texture* csxTexture = nullptr;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static std::array<bool, 5> activeButtons = {true, false, false, false, false};
    static ImGuiTextFilter textFilter;

    ImGui::Begin("CSX Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilter.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(csxFiles.size()); ++i)
            {
                if (textFilter.PassFilter(csxFiles[i].c_str())
                    && ImGui::Selectable(csxFiles[i].c_str(), selectedIndex == i))
                {
                    selectedIndex = i;

                    if (csxTexture)
                        SDL_DestroyTexture(csxTexture);

                    TextureLoader::loadTextureFromCsxFile(std::format("{}/{}", rootDirectory, csxFiles[i]).c_str(), renderer, &csxTexture);
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();

        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_HorizontalScrollbar);
        if (csxTexture) {
            ImGui::Text("%dx%d", csxTexture->w, csxTexture->h);
            ImGui::ImageWithBg((ImTextureID)csxTexture, ImVec2(csxTexture->w, csxTexture->h), ImVec2(0, 0), ImVec2(1, 1), bgColor);
        }
        ImGui::EndChild();

        if (ImGui::RadioButton("Transparent", activeButtons[0])) {
            bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            activeButtons.fill(false);
            activeButtons[0] = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Black", activeButtons[1])) {
            bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            activeButtons.fill(false);
            activeButtons[1] = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Gray", activeButtons[2])) {
            bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            activeButtons.fill(false);
            activeButtons[2] = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("White", activeButtons[3])) {
            bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            activeButtons.fill(false);
            activeButtons[3] = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Custom", activeButtons[4])) {
            activeButtons.fill(false);
            activeButtons[4] = true;
        }
        ImGui::SameLine();
        if (activeButtons[4]) {
            ImGui::ColorEdit4("Color", (float*)&bgColor, ImGuiColorEditFlags_NoInputs);
        }

        ImGui::EndGroup();
    }

    ImGui::End();

    return true;
}
