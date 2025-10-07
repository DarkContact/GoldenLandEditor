#include "SdbViewer.h"

#include <format>

#include "imgui.h"
#include "parsers/SDB_Parser.h"

#include "utils/TracyProfiler.h"

void SdbViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files)
{
    Tracy_ZoneScopedN("SdbViewer::update");

    static int selectedIndex = -1;
    static SDB_Data sdbRecords;
    static ImGuiTextFilter textFilterFile;
    static ImGuiTextFilter textFilterString;

    bool needResetScroll = false;

    ImGui::Begin("SDB Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilterFile.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(files.size()); ++i)
            {
                if (textFilterFile.PassFilter(files[i].c_str())
                    && ImGui::Selectable(files[i].c_str(), selectedIndex == i))
                {
                    selectedIndex = i;

                    SDB_Parser parser(std::format("{}/{}", rootDirectory, files[i]));
                    sdbRecords = parser.parse();

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
        textFilterString.Draw();
        if (!sdbRecords.strings.empty()) {
            if (ImGui::BeginTable("content", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Текст");
                ImGui::TableHeadersRow();

                for (const auto& [id, text] : sdbRecords.strings) {
                    if (textFilterString.PassFilter(std::format("{} {}", id, text).c_str())) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", id);

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", text.c_str());
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        sdbRecords.strings.clear();
        textFilterFile.Clear();
        textFilterString.Clear();
    }
}
