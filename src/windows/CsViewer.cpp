#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "utils/DebugLog.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

void CsViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    Tracy_ZoneScoped;

    static int selectedIndex = -1;
    static ImGuiTextFilter textFilterFile;
    static ImGuiTextFilter textFilterString;
    static CS_Data csData;
    static SDB_Data sdbDialogs;
    static std::string csError;
    static std::vector<bool> funcNodes;
    static bool showOnlyFunctions = false;
    static bool showDialogPhrases = true;
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
                    funcNodes.clear();

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

                    funcNodes.resize(csData.nodes.size(), false);
                    for (size_t i = 0; i < csData.nodes.size(); ++i) {
                        const auto& node = csData.nodes[i];
                        if (node.opcode == 48) {
                            funcNodes[i] = true;
                            for (int j = 0; j < node.child.size(); ++j) {
                                int32_t idx = node.child[j];
                                if (idx == -1) break;

                                funcNodes[++i] = true;
                            }
                        }
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
        ImGui::BeginChild("right pane");
        textFilterString.Draw();

        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_HorizontalScrollbar);

            if (!csData.nodes.empty()) {
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                int counter = 0;
                ImGui::PushFont(NULL, 15.0f);
                for (const auto& node : csData.nodes) {
                    std::string nodeInfo = csNodeString(node, sdbDialogs, showDialogPhrases);

                    if (showOnlyFunctions && funcNodes[counter]
                        || !showOnlyFunctions) {
                        if (textFilterString.PassFilter(nodeInfo.c_str()))
                            ImGui::Text("[i:%d] %s", counter, nodeInfo.c_str());
                    }
                    ++counter;
                }
                ImGui::PopFont();
            } else if (selectedIndex >= 0) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", csError.c_str());
            }
            ImGui::EndChild();
        }

        ImGui::Checkbox("Funcs only", &showOnlyFunctions);
        ImGui::SameLine();
        ImGui::Checkbox("Dialog phrases", &showDialogPhrases);

        ImGui::EndChild();
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        csError.clear();
        textFilterFile.Clear();
        textFilterString.Clear();
        sdbDialogs = {}; // FIXME: Не чистится если сменили RootDirectory
        funcNodes.clear();
        onceWhenOpen = false;
    }
}

const char* CsViewer::opcodeStr(int32_t opcode) {
    switch (opcode) {
        case 0: return "||";
        case 1: return "^^";
        case 2: return "&&";
        case 3: return "|";
        case 4: return "^";
        case 5: return "&";
        case 6: return "!=";
        case 7: return "==";
        case 8: return ">=";
        case 9: return "<=";
        case 10: return ">";
        case 11: return "<";
        case 12: return "<<";
        case 13: return ">>";
        case 14: return "+";
        case 15: return "-";
        case 16: return "*";
        case 17: return "/";
        case 18: return "%";
        case 19: return "~";
        case 20: return "!";
        case 21: return "real_var";
        case 22: return "str";
        case 23: return "str_var";
        case 24: return "real";
        case 48: return "func";
        case 49: return "jmp";
        case 50: return "if_call";
        default: return "unk";
    }
}

const char* CsViewer::funcStr(double value) {
    if (value == 16777220) return "D_Say";
    if (value == 16777221) return "D_CloseDialog";
    if (value == 16777222) return "D_Answer";
    if (value == 16777223) return "D_PlaySound";
    if (value == 33554432) return "LE_CastEffect";
    if (value == 50331650) return "RS_SetTribesRelation";
    if (value == 67108864) return "RS_GetPersonParameterI";
    if (value == 67108865) return "RS_SetPersonParameterI";
    if (value == 67108866) return "RS_AddPerson_1";
    if (value == 67108867) return "RS_AddPerson_2";
    if (value == 67108869) return "RS_AddExp";
    if (value == 67108870) return "RS_IsPersonExistsI";
    if (value == 67108871) return "RS_DelPerson";
    if (value == 83886080) return "RS_TestPersonHasItem";
    if (value == 83886081) return "RS_PersonTransferItemI";
    if (value == 83886082) return "RS_GetItemCountI";
    if (value == 83886085) return "RS_PersonRemoveItem";
    if (value == 117440513) return "RS_StageEnable";
    if (value == 117440514) return "RS_QuestEnable";
    if (value == 117440515) return "RS_StageComplete";
    if (value == 117440522) return "RS_GetRandMinMaxI";
    if (value == 117440526) return "RS_GetDialogEnabled"; // Связано с убеждением (передаётся значение 2), dlg2 == 1 прошли проверку
    return "unk";
}

std::string CsViewer::csNodeString(const CS_Node& node, const SDB_Data& sdbDialogs, bool showDialogPhrases) {
    std::string additionInfo;
    if (node.opcode >= 0 && node.opcode <= 20) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    } else if (node.opcode == 21 || node.opcode == 24) {
        if (showDialogPhrases && node.opcode == 24) {
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
        std::string_view funcStr = CsViewer::funcStr(node.value);
        if (funcStr == "unk") {
            additionInfo = std::format("val: {} [{}], c: {}, d: {}, childs: [{}]", node.value, funcStr, node.c, node.d, StringUtils::trimRight(childInfo));
        } else {
            additionInfo = std::format("val: [{}], c: {}, d: {}, childs: [{}]", funcStr, node.c, node.d, StringUtils::trimRight(childInfo));
        }
    } else if (node.opcode == 49) {
        additionInfo = std::format("c: {}, d: {}", node.c, node.d);
    } else if (node.opcode == 50) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    }

    return std::format("Opcode: {} [{}] | {}", node.opcode, CsViewer::opcodeStr(node.opcode), additionInfo);
}
