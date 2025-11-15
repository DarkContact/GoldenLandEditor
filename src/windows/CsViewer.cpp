#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "parsers/CS_Parser.h"

void CsViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    static int selectedIndex = -1;
    static ImGuiTextFilter textFilter;
    static CS_Data csData;
    static std::string csError;

    bool needResetScroll = false;

    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
    ImGui::Begin("CS Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilter.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(csFiles.size()); ++i)
            {
                if (textFilter.PassFilter(csFiles[i].c_str())
                    && ImGui::Selectable(csFiles[i].c_str(), selectedIndex == i))
                {
                    selectedIndex = i;

                    csError.clear();
                    csData.nodes.clear();

                    std::string csPath = std::format("{}/{}", rootDirectory, csFiles[i]);
                    CS_Parser::parse(csPath, csData, &csError);

                    needResetScroll = true;
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    if (!csData.nodes.empty()) {
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            ImGui::EndChild();
        }
        ImGui::EndGroup();
    } else if (selectedIndex >= 0) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", csError.c_str());
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        csError.clear();
        textFilter.Clear();
    }
}
