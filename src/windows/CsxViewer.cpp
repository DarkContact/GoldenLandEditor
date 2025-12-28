#include "CsxViewer.h"

#include <format>

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_render.h>
#include "imgui.h"

#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"
#include "Texture.h"

CsxViewer::CsxViewer() {}

void CsxViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles)
{
    Tracy_ZoneScoped;
    m_saveDialogData.renderer = renderer;

    if (showWindow && !csxFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("CSX Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(400, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            m_textFilter.Draw();
            ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(csxFiles.size()); ++i)
                {
                    if (m_textFilter.PassFilter(csxFiles[i].c_str())
                        && ImGui::Selectable(csxFiles[i].c_str(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        m_csxTextures.clear();
                        TextureLoader::loadTexturesFromCsxFile(std::format("{}/{}", rootDirectory, csxFiles[i]), renderer, m_csxTextures, &m_csxTextureError);

                        needResetScroll = true;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        if (!m_csxTextures.empty()) {
            ImGui::BeginGroup();
            {
                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                ImVec2 originalSpacing = ImGui::GetStyle().ItemSpacing;
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(originalSpacing.x, 0)); // убрать вертикальный отступ (когда отображаем >= 2 текстуры подряд)

                int csxTextureWidth = 0;
                int csxTextureHeight = 0;
                for (const auto& csxTexture : m_csxTextures) {
                    csxTextureWidth = csxTexture->w;
                    csxTextureHeight += csxTexture->h;
                    ImGui::ImageWithBg((ImTextureID)csxTexture.get(), ImVec2(csxTexture->w, csxTexture->h), ImVec2(0, 0), ImVec2(1, 1), m_bgColor);
                }

                ImGui::PopStyleVar();

                ImGui::EndChild();

                ImGui::Text("%dx%d", csxTextureWidth, csxTextureHeight);
                ImGui::SameLine();

                if (ImGui::Button("Save as BMP")) {
                    std::string_view filename = StringUtils::filename(csxFiles[m_selectedIndex]);
                    filename.remove_suffix(4);
                    std::string savePath = std::format("{}/{}.bmp", rootDirectory, filename);

                    m_saveDialogData.csxPath = std::format("{}/{}", rootDirectory, csxFiles[m_selectedIndex]);
                    SDL_ShowSaveFileDialog([] (void* userdata, const char* const* filelist, int filter) {
                        CsxViewer* self = static_cast<CsxViewer*>(userdata);
                        if (!filelist) {
                            LogFmt("Folder dialog error: {}", SDL_GetError());
                            return;
                        } else if (!*filelist) {
                            Log("Dialog was canceled");
                            // Dialog was canceled.
                            return;
                        } else if ((*filelist)[0] == '\0') {
                            Log("Filelist empty");
                            return;
                        }

                        std::string_view bmpPath(*filelist);
                        std::string error;
                        if (!TextureLoader::saveCsxAsBmpFile(self->m_saveDialogData.csxPath, bmpPath, self->m_saveDialogData.renderer, &error)) {
                            LogFmt("TextureLoader::saveCsxAsBmpFile error: {}", error);
                        }
                    }, this, SDL_GetRenderWindow(renderer), NULL, 0, savePath.c_str());
                }

                if (ImGui::RadioButton("Transparent", m_activeButtonIndex == 0)) {
                    m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
                    m_activeButtonIndex = 0;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Black", m_activeButtonIndex == 1)) {
                    m_bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    m_activeButtonIndex = 1;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Gray", m_activeButtonIndex == 2)) {
                    m_bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                    m_activeButtonIndex = 2;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("White", m_activeButtonIndex == 3)) {
                    m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    m_activeButtonIndex = 3;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Custom", m_activeButtonIndex == 4)) {
                    m_activeButtonIndex = 4;
                }
                ImGui::SameLine();
                if (m_activeButtonIndex == 4) {
                    ImGui::ColorEdit4("Color", (float*)&m_bgColor, ImGuiColorEditFlags_NoInputs);
                }
            }
            ImGui::EndGroup();
        } else if (m_selectedIndex >= 0) {
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_csxTextureError.c_str());
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_csxTextures.clear();
        m_csxTextureError.clear();
        m_textFilter.Clear();
        m_onceWhenClose = true;
    }
}
