#include "SdbViewer.h"

#include <format>

#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

SdbViewer::SdbViewer() {

}

void SdbViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files)
{
    Tracy_ZoneScoped;
    
    if (showWindow && !files.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("SDB Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            m_textFilterFile.Draw();
            ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(files.size()); ++i)
                {
                    if (m_textFilterFile.PassFilter(files[i].c_str())
                        && ImGui::Selectable(files[i].c_str(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        m_sdbRecords.strings.clear();

                        std::string error;
                        if (!SDB_Parser::parse(std::format("{}/{}", rootDirectory, files[i]), m_sdbRecords, &error)) {
                            LogFmt("SdbViewer error: {}", error);
                        }

                        needResetScroll = true;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        {
            ImGui::BeginChild("item view");
            m_textFilterString.Draw();
            if (!m_sdbRecords.strings.empty()) {
                if (ImGui::BeginTable("content", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {
                    if (needResetScroll) {
                        ImGui::SetScrollX(0.0f);
                        ImGui::SetScrollY(0.0f);
                    }

                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Текст");
                    ImGui::TableHeadersRow();

                    for (const auto& [id, text] : m_sdbRecords.strings) {
                        char filterBuffer[4096];
                        StringUtils::formatToBuffer(filterBuffer, "{} {}", id, text);

                        if (m_textFilterString.PassFilter(filterBuffer)) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", id);

                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(text.c_str());
                        }
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_sdbRecords.strings.clear();
        m_textFilterFile.Clear();
        m_textFilterString.Clear();
        m_onceWhenClose = true;
    }
}
