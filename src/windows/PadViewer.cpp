#include "PadViewer.h"

#include "utils/TracyProfiler.h"

PadViewer::PadViewer() {}

void PadViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& padFiles)
{
    Tracy_ZoneScoped;

    if (showWindow && !padFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("PAD Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(400, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            m_textFilter.Draw();
            ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(padFiles.size()); ++i)
                {
                    if (m_textFilter.PassFilter(padFiles[i].c_str())
                        && ImGui::Selectable(padFiles[i].c_str(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        //m_csxTextures.clear();
                        //TextureLoader::loadTexturesFromCsxFile(std::format("{}/{}", rootDirectory, csxFiles[i]), renderer, m_csxTextures, &m_csxTextureError);

                        needResetScroll = true;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_onceWhenClose = true;
    }
}

bool PadViewer::isAnimating() const {
    return true;
}
