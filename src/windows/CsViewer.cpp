#include "CsViewer.h"

#include <format>

#include "imgui.h"

#include "Types.h"

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"

#include "utils/DebugLog.h"
#include "utils/TracyProfiler.h"

#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

#include "CsExecutor.h"
#include "CsExecutorViewer.h"

void CsViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    Tracy_ZoneScoped;
    static int selectedIndex = -1;
    static ImGuiTextFilter textFilterFile;
    static ImGuiTextFilter textFilterString;
    static CS_Data csData;
    static SDB_Data sdbDialogs;
    static UMapStringVar_t globalVars;
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

            std::string varsPath = std::format("{}/scripts/dialogs_special/zlato_vars.scr", rootDirectory);
            if (!CsExecutor::readGlobalVariables(varsPath, globalVars, &error))
                LogFmt("Load zlato_vars.scr error: {}", error);
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
                        std::string nodeInfo = node.toString((isDialogPhrase && showDialogPhrases), sdbDialogs.strings);

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

        CsExecutorViewer::update(showExecuteWindow, needUpdate, csData.nodes, globalVars);
    }

    // Очистка
    if (!showWindow && !onceWhenClose) {
        selectedIndex = -1;
        csError.clear();
        csData.nodes.clear();
        textFilterFile.Clear();
        textFilterString.Clear();
        sdbDialogs = {};
        globalVars.clear();
        funcNodes.clear();
        onceWhenOpen = false;
        onceWhenClose = true;
    }
}
