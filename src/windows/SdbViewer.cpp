#include "SdbViewer.h"

#include <format>

#include "imgui.h"
#include "parsers/SDB_Parser.h"

bool SdbViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files)
{
    if (files.empty()) return false;

    static int selectedIndex = -1;
    static SDB_Data sdbRecords;
    static ImGuiTextFilter textFilter;

    ImGui::Begin("SDB Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilter.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(files.size()); ++i)
            {
                if (textFilter.PassFilter(files[i].c_str())
                    && ImGui::Selectable(files[i].c_str(), selectedIndex == i))
                {
                    selectedIndex = i;

                    SDB_Parser parser(std::format("{}/{}", rootDirectory, files[i]));
                    sdbRecords = parser.parse();
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    {
        ImGui::BeginChild("item view", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);
        if (!sdbRecords.strings.empty()) {
            //if (ImGui::BeginTable("content", 2)) {
                for (const auto& [id, text] : sdbRecords.strings) {
                    ImGui::Text("%d: %s", id, text.c_str());
                    // ImGui::Text("%d", id);
                    // ImGui::TableNextColumn();
                    // ImGui::Text("%s", text.c_str());
                    //ImGui::TableNextRow();
                }
                //ImGui::EndTable();
            //}
        }
        ImGui::EndChild();
    }

    ImGui::End();

    return true;
}
