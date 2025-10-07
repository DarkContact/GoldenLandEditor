#include "CsxViewer.h"

#include <format>

#include "utils/TextureLoader.h"
#include "Texture.h"
#include "imgui.h"

#include "utils/TracyProfiler.h"

void CsxViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles)
{
    Tracy_ZoneScopedN("CsxViewer::update");

    static int selectedIndex = -1;
    static std::vector<Texture> csxTextures;
    static std::string csxTextureError;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static int activeButtonIndex = 0;
    static ImGuiTextFilter textFilter;

    bool needResetScroll = false;

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

                    csxTextures.clear();
                    TextureLoader::loadTexturesFromCsxFile(std::format("{}/{}", rootDirectory, csxFiles[i]).c_str(), renderer, csxTextures, &csxTextureError);

                    needResetScroll = true;
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    if (!csxTextures.empty()) {
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            ImVec2 originalSpacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(originalSpacing.x, 0)); // убрать вертикальный отступ

            int csxTextureWidth = 0;
            int csxTextureHeight = 0;
            for (const auto& csxTexture : csxTextures) {
                csxTextureWidth = csxTexture->w;
                csxTextureHeight += csxTexture->h;
                ImGui::ImageWithBg((ImTextureID)csxTexture.get(), ImVec2(csxTexture->w, csxTexture->h), ImVec2(0, 0), ImVec2(1, 1), bgColor);
            }

            ImGui::PopStyleVar();

            ImGui::EndChild();

            ImGui::Text("%dx%d", csxTextureWidth, csxTextureHeight);

            if (ImGui::RadioButton("Transparent", activeButtonIndex == 0)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
                activeButtonIndex = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Black", activeButtonIndex == 1)) {
                bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                activeButtonIndex = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Gray", activeButtonIndex == 2)) {
                bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                activeButtonIndex = 2;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("White", activeButtonIndex == 3)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                activeButtonIndex = 3;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Custom", activeButtonIndex == 4)) {
                activeButtonIndex = 4;
            }
            ImGui::SameLine();
            if (activeButtonIndex == 4) {
                ImGui::ColorEdit4("Color", (float*)&bgColor, ImGuiColorEditFlags_NoInputs);
            }
        }
        ImGui::EndGroup();
    } else if (selectedIndex > 0) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", csxTextureError.c_str());
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        csxTextures.clear();
        csxTextureError.clear();
        textFilter.Clear();
    }
}
