#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "utils/StringUtils.h"
#include "parsers/CS_Parser.h"

static std::string csNodeString(const CS_Node& node) {
    std::string additionInfo;
    if (node.opcode >= 0 && node.opcode <= 20) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    } else if (node.opcode == 21 || node.opcode == 24) {
        additionInfo = std::format("val: {}", node.value);
    } else if (node.opcode == 22 || node.opcode == 23) {
        additionInfo = std::format("txt: {}", node.text);
    } else if (node.opcode == 48) {
        std::string childInfo;
        for (int j = 0; j < 9; j++) {
            int32_t idx = node.child[j];
            if (idx == -1) break;
            childInfo += std::format("{} ", idx);
        }
        additionInfo = std::format("val: {}, c: {}, d: {}, childs: [{}]", node.value, node.c, node.d, StringUtils::trimRight(childInfo));
    } else if (node.opcode == 49) {
        additionInfo = std::format("c: {}, d: {}", node.c, node.d);
    } else if (node.opcode == 50) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    }

    return std::format("Opcode: {} [{}] | {}", node.opcode, CS_Parser::opcodeStr(node.opcode), additionInfo);
}

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
            ImGui::BeginChild("item view", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            int counter = 0;
            for (const auto& node : csData.nodes) {
                ImGui::PushFont(NULL, 15.0f);
                ImGui::Text("[i:%d] %s", counter, csNodeString(node).c_str());
                ImGui::PopFont();

                ++counter;
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
