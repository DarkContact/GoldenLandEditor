#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"

#include "utils/DebugLog.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

#include "CsExecutorViewer.h"

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
    static bool showExecuteWindow = false;

    static bool onceWhenOpen = false;
    static bool onceWhenClose = true;

    static const ImVec4 defaultTextColor(1.0f, 1.0f, 1.0f, 1.0f);
    static const ImVec4 funcTextColor(1.0f, 0.92f, 0.5f, 1.0f);

    if (showWindow && !csFiles.empty()) {
        onceWhenClose = false;
        bool needResetScroll = false;
        bool needUpdate = false;

        if (!onceWhenOpen) {
            std::string error;
            std::string sdbPath = std::format("{}/sdb/dialogs/dialogsphrases.sdb", rootDirectory);
            if (!SDB_Parser::parse(sdbPath, sdbDialogs, &error))
                LogFmt("Load dialogsphrases.sdb error: {}", error);
            onceWhenOpen = true;
        }

        bool isOpenedWindow = true;
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
                            if (node.opcode == kFunc) {
                                funcNodes[i] = true;
                                for (int j = 0; j < node.args.size(); ++j) {
                                    int32_t idx = node.args[j];
                                    if (idx == -1) break;

                                    funcNodes[++i] = true;
                                }
                            }
                        }

                        needResetScroll = true;
                        needUpdate = true;
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

                    ImGuiStyle& style = ImGui::GetStyle();
                    ImGui::PushFont(NULL, style.FontSizeBase - 1.0f);
                    for (size_t i = 0; i < csData.nodes.size(); ++i) {
                        const CS_Node* prevNode = nullptr;
                        if (i > 0) {
                            prevNode = &csData.nodes[i - 1];
                        }

                        bool isDialogPhrase = false;
                        if (prevNode) {
                            if (prevNode->opcode == kStringVarName) {
                                if (prevNode->text == "LastPhrase" || prevNode->text == "LastAnswer")
                                    isDialogPhrase = true;
                            }

                            if (prevNode->opcode == kFunc) {
                                std::string_view funcStr = csFuncToString(prevNode->value);
                                if (funcStr == "D_Say" || funcStr == "D_Answer")
                                    isDialogPhrase = true;
                            }
                        }

                        const CS_Node& node = csData.nodes[i];
                        std::string nodeInfo = csNodeString(node, sdbDialogs, isDialogPhrase && showDialogPhrases);

                        if (showOnlyFunctions && funcNodes[i]
                            || !showOnlyFunctions) {
                            if (textFilterString.PassFilter(nodeInfo.c_str())) {
                                bool isFunc = node.opcode == kFunc;
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
            ImGui::SameLine();

            if (!csData.nodes.empty()) {
                ImGui::BeginDisabled(showExecuteWindow);
                if (ImGui::Button("Execute")) {
                    showExecuteWindow = true;
                }
                ImGui::EndDisabled();
            }

            ImGui::EndChild();
        }

        ImGui::End();

        CsExecutorViewer::update(showExecuteWindow, needUpdate, rootDirectory, csData.nodes);
    }

    // Очистка
    if (!showWindow && !onceWhenClose) {
        selectedIndex = -1;
        csError.clear();
        csData.nodes.clear();
        textFilterFile.Clear();
        textFilterString.Clear();
        sdbDialogs = {};
        funcNodes.clear();
        onceWhenOpen = false;
        onceWhenClose = true;
    }
}

std::string CsViewer::csNodeString(const CS_Node& node, const SDB_Data& sdbDialogs, bool showDialogPhrases) {
    std::string additionInfo;
    if (node.opcode >= 0 && node.opcode <= 20) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    } else if (node.opcode == kNumberLiteral || node.opcode == kNumberVarName) {
        if (showDialogPhrases && node.opcode == kNumberLiteral) {
            auto it = sdbDialogs.strings.find(node.value);
            if (it != sdbDialogs.strings.cend()) {
                additionInfo = std::format("val: {} [{}]", node.value, it->second);
            } else {
                additionInfo = std::format("val: {}", node.value);
            }
        } else {
            additionInfo = std::format("val: {}", node.value);
        }
    } else if (node.opcode == kStringLiteral || node.opcode == kStringVarName) {
        additionInfo = std::format("txt: {}", StringUtils::decodeWin1251ToUtf8(node.text));
    } else if (node.opcode == kFunc) {
        std::string argsInfo;
        for (int j = 0; j < node.args.size(); j++) {
            int32_t idx = node.args[j];
            if (idx == -1) break;
            argsInfo += std::format("{} ", idx);
        }
        std::string_view funcStr = csFuncToString(node.value);
        additionInfo = std::format("val: [{}], c: {}, d: {}, args: [{}]", funcStr, node.c, node.d, StringUtils::trimRight(argsInfo));
    } else if (node.opcode == kJmp) {
        additionInfo = std::format("c: {}, d: {}", node.c, node.d);
    } else if (node.opcode == kAssign) {
        additionInfo = std::format("a: {}, b: {}, c: {}, d: {}", node.a, node.b, node.c, node.d);
    }

    return std::format("Opcode: {} [{}] | {}", node.opcode, csOpcodeToString(node.opcode), additionInfo);
}
