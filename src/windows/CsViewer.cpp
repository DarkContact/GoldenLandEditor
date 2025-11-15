#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "utils/DebugLog.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

static std::string csNodeString(const CS_Node& node, const SDB_Data& sdbDialogs) {
    std::string additionInfo;
    if (node.opcode >= 0 && node.opcode <= 20) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    } else if (node.opcode == 21 || node.opcode == 24) {
        if (node.opcode == 24) {
            auto it = sdbDialogs.strings.find(node.value);
            if (it != sdbDialogs.strings.cend()) {
                additionInfo = std::format("val: {} [{}]", node.value, it->second);
            } else {
                additionInfo = std::format("val: {}", node.value);
            }
        } else {
            additionInfo = std::format("val: {}", node.value);
        }
    } else if (node.opcode == 22 || node.opcode == 23) {
        additionInfo = std::format("txt: {}", node.text);
    } else if (node.opcode == 48) {
        std::string childInfo;
        for (int j = 0; j < node.child.size(); j++) {
            int32_t idx = node.child[j];
            if (idx == -1) break;
            childInfo += std::format("{} ", idx);
        }
        additionInfo = std::format("val: {} [{}], c: {}, d: {}, childs: [{}]", node.value, CS_Parser::funcStr(node.value), node.c, node.d, StringUtils::trimRight(childInfo));
    } else if (node.opcode == 49) {
        additionInfo = std::format("c: {}, d: {}", node.c, node.d);
    } else if (node.opcode == 50) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    }

    return std::format("Opcode: {} [{}] | {}", node.opcode, CS_Parser::opcodeStr(node.opcode), additionInfo);
}

void CsViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    Tracy_ZoneScoped;

    static int selectedIndex = -1;
    static ImGuiTextFilter textFilterFile;
    static ImGuiTextFilter textFilterString;
    static CS_Data csData;
    static SDB_Data sdbDialogs;
    static std::string csError;
    static bool onceWhenOpen = false;

    bool needResetScroll = false;

    if (!onceWhenOpen) {
        std::string error;
        std::string sdbPath = std::format("{}/sdb/dialogs/dialogsphrases.sdb", rootDirectory);
        if (!SDB_Parser::parse(sdbPath, sdbDialogs, &error))
            LogFmt("Load dialogsphrases.sdb error: {}", error);
        onceWhenOpen = true;
    }

    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
    ImGui::Begin("CS Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilterFile.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(csFiles.size()); ++i)
            {
                if (textFilterFile.PassFilter(csFiles[i].c_str())
                    && ImGui::Selectable(csFiles[i].c_str(), selectedIndex == i))
                {
                    selectedIndex = i;

                    csError.clear();
                    csData.nodes.clear();

                    std::string csPath = std::format("{}/{}", rootDirectory, csFiles[i]);
                    CS_Parser::parse(csPath, csData, &csError);

                    // // Отладочный вывод
                    // std::string csDataInfo;
                    // int counter = 0;
                    // for (const auto& node : csData.nodes) {
                    //     std::string nodeInfo = csNodeString(node, {});
                    //     csDataInfo += std::format("[i:{}] {}\n", counter, nodeInfo);
                    //     ++counter;
                    // }
                    // LogFmt("File: {}, Data: {}", csFiles[i], csDataInfo);
                    // // ---

                    needResetScroll = true;
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    {
        ImGui::BeginChild("right pane");
        textFilterString.Draw();

            ImGui::BeginChild("item view", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);

            if (!csData.nodes.empty()) {
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                int counter = 0;
                ImGui::PushFont(NULL, 15.0f);
                for (const auto& node : csData.nodes) {
                    std::string nodeInfo = csNodeString(node, sdbDialogs);
                    if (textFilterString.PassFilter(nodeInfo.c_str())) {
                        ImGui::Text("[i:%d] %s", counter, nodeInfo.c_str());
                    }
                    ++counter;
                }
                ImGui::PopFont();
            } else if (selectedIndex >= 0) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", csError.c_str());
            }
            ImGui::EndChild();

        ImGui::EndChild();
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        csError.clear();
        textFilterFile.Clear();
        textFilterString.Clear();
        sdbDialogs = {};
        onceWhenOpen = false;
    }
}
