#include "CsxViewer.h"

#include <format>

#include "TextureLoader.h"
#include "imgui.h"

bool CsxViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles)
{
    if (csxFiles.empty()) return false;

    static int selectedIndex = 0;
    static SDL_Texture* csxTexture = nullptr;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);

    ImGui::Begin("CSX Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        for (int i = 0; i < static_cast<int>(csxFiles.size()); ++i)
        {
            if (ImGui::Selectable(csxFiles[i].c_str(), selectedIndex == i)) {
                selectedIndex = i;

                if (csxTexture)
                    SDL_DestroyTexture(csxTexture);

                TextureLoader::loadTextureFromCsxFile(std::format("{}/{}", rootDirectory, csxFiles[i]).c_str(), renderer, &csxTexture);
            }
        }
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();

        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_HorizontalScrollbar);
        if (csxTexture)
            ImGui::ImageWithBg((ImTextureID)csxTexture, ImVec2((float)csxTexture->w, (float)csxTexture->h), ImVec2(0, 0), ImVec2(1, 1), bgColor);
        ImGui::EndChild();

        if (ImGui::Button("Transparent")) {
            bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Black")) {
            bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("White")) {
            bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        ImGui::EndGroup();
    }

    ImGui::End();

    return true;
}
