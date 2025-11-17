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

    static const ImVec4 defaultTextColor(1.0f, 1.0f, 1.0f, 1.0f);
    static const ImVec4 funcTextColor(1.0f, 0.92f, 0.5f, 1.0f);

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

                    // Заполнение данных для фильтрации функций
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

                ImGui::PushFont(NULL, 15.0f);
                for (size_t i = 0; i < csData.nodes.size(); ++i) {
                    const CS_Node* prevNode = nullptr;
                    if (i > 0) {
                        prevNode = &csData.nodes[i - 1];
                    }

                    bool isDialogPhrase = false;
                    if (prevNode) {
                        if (prevNode->opcode == 23) {
                            if (prevNode->text == "LastPhrase" || prevNode->text == "LastAnswer")
                                isDialogPhrase = true;
                        }

                        if (prevNode->opcode == 48) {
                            std::string_view funcStr = CsViewer::funcStr(prevNode->value);
                            if (funcStr == "D_Say" || funcStr == "D_Answer")
                                isDialogPhrase = true;
                        }
                    }

                    const CS_Node& node = csData.nodes[i];
                    std::string nodeInfo = csNodeString(node, sdbDialogs, isDialogPhrase && showDialogPhrases);

                    if (showOnlyFunctions && funcNodes[i]
                        || !showOnlyFunctions) {
                        if (textFilterString.PassFilter(nodeInfo.c_str())) {
                            bool isFunc = node.opcode == 48;
                            ImGui::TextColored(isFunc ? funcTextColor : defaultTextColor, "[i:%zu] %s", i, nodeInfo.c_str());
                        }
                    }
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
        csData.nodes.clear();
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

// SERVER.DLL [14042464]
const char* CsViewer::funcStr(double value) {
    if (value == 0) return "RS_GetPersonParameterI (0)";
    if (value == 0x1000000) return "Exit";
    if (value == 0x1000001) return "Signal";
    if (value == 0x1000002) return "Console";
    if (value == 0x1000003) return "Cmd";
    if (value == 0x1000004) return "D_Say";
    if (value == 0x1000005) return "D_CloseDialog";
    if (value == 0x1000006) return "D_Answer";
    if (value == 0x1000007) return "D_PlaySound";
    if (value == 0x2000000) return "LE_CastEffect";
    if (value == 0x2000001) return "LE_DelEffect";
    if (value == 0x2000002) return "LE_CastMagic";
    if (value == 0x3000000) return "WD_LoadArea";
    if (value == 0x3000001) return "WD_SetCellsGroupFlag";
    if (value == 0x3000002) return "RS_SetTribesRelation";
    if (value == 0x3000003) return "RS_GetTribesRelation";
    if (value == 0x3000004) return "RS_StartDialog";
    if (value == 0x3000005) return "WD_SetVisible";
    if (value == 0x3000006) return "C_FINISHED";
    if (value == 0x3000007) return "WD_TitlesAndLoadArea";
    if (value == 0x3000008) return "C_TitlesAndFINISHED";
    if (value == 0x4000000) return "RS_GetPersonParameterI";
    if (value == 0x4000001) return "RS_SetPersonParameterI";
    if (value == 0x4000002) return "RS_AddPerson_1";
    if (value == 0x4000003) return "RS_AddPerson_2";
    if (value == 0x4000004) return "RS_IsPersonExistsI";
    if (value == 0x4000005) return "RS_AddExp";
    if (value == 0x4000006) return "RS_DelPerson";
    if (value == 0x4000007) return "RS_AddToHeroPartyName";
    if (value == 0x4000008) return "RS_RemoveFromHeroPartyName";
    if (value == 0x4000009) return "RS_TestHeroHasPartyName";
    if (value == 0x400000A) return "RS_AllyCmd";
    if (value == 0x400000B) return "RS_ShowMessage";
    if (value == 0x5000000) return "RS_TestPersonHasItem";
    if (value == 0x5000001) return "RS_PersonTransferItemI";
    if (value == 0x5000002) return "RS_GetItemCountI";
    if (value == 0x5000003) return "RS_PersonTransferAllItemsI";
    if (value == 0x5000004) return "RS_PersonAddItem";
    if (value == 0x5000005) return "RS_PersonRemoveItem";
    if (value == 0x5000006) return "RS_PersonAddItemToTrade";
    if (value == 0x5000007) return "RS_PersonRemoveItemToTrade";
    if (value == 0x5000008) return "RS_GetMoney";
    if (value == 0x6000000) return "RS_GetDayOrNight";
    if (value == 0x6000001) return "RS_GetCurrentTimeOfDayI";
    if (value == 0x6000002) return "RS_GetDaysFromBeginningI";
    if (value == 0x6000003) return "RS_AddTime";
    if (value == 0x7000000) return "RS_QuestComplete";
    if (value == 0x7000001) return "RS_StageEnable";
    if (value == 0x7000002) return "RS_QuestEnable";
    if (value == 0x7000003) return "RS_StageComplete";
    if (value == 0x7000004) return "RS_StorylineQuestEnable";
    if (value == 0x7000005) return "RS_SetEvent";
    if (value == 0x7000006) return "RS_GetEvent";
    if (value == 0x7000007) return "RS_ClearEvent";
    if (value == 0x7000008) return "RS_SetLocationAccess";
    if (value == 0x7000009) return "RS_EnableTrigger";
    if (value == 0x700000A) return "RS_GetRandMinMaxI";
    if (value == 0x700000B) return "RS_SetWeather";
    if (value == 0x700000C) return "RS_SetSpecialPerk";
    if (value == 0x700000D) return "RS_PassToTradePanel";
    if (value == 0x700000E) return "RS_GetDialogEnabled";
    if (value == 0x700000F) return "RS_SetUndeadState";
    if (value == 0x7000010) return "RS_GlobalMap";
    if (value == 0x7000013) return "RS_SetInjured";
    if (value == 0x7000014) return "RS_SetDoorState";
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
            additionInfo = std::format("val: {:X} [{}], c: {}, d: {}, childs: [{}]", (int)node.value, funcStr, node.c, node.d, StringUtils::trimRight(childInfo));
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
