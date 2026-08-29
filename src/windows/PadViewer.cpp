#include "PadViewer.h"

#include "utils/TracyProfiler.h"
#include <format>

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

                        m_padData = PAD_Parser::parse(std::format("{}/{}", rootDirectory, padFiles[i]), &m_error);

                        needResetScroll = true;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        if (!padFiles.empty() && m_padData) {
            ImGui::BeginGroup();

            ImGui::Text("Size: %zu, Mask: %u", m_padData->animationData.size(), m_padData->animationMask);
            for (auto animation : m_padData->animations) {
                ImGui::Text("%s", animationTypeMaskToString(animation).data());
            }

            ImGui::EndGroup();
        } else if (m_selectedIndex >= 0) {
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_error.c_str());
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_onceWhenClose = true;
        m_textFilter.Clear();
        m_error.clear();
        m_padData = {};
    }
}

bool PadViewer::isAnimating() const {
    return true;
}
