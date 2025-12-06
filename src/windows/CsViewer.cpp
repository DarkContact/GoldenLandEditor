#include "CsViewer.h"

#include <format>

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"
#include "utils/DebugLog.h"
#include "utils/TracyProfiler.h"
#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"

#include "CsExecutor.h"
#include "CsExecutorViewer.h"

CsViewer::CsViewer() {

}

void CsViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles)
{
    Tracy_ZoneScoped;
    if (showWindow && !csFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;
        bool needUpdate = false;

        if (!m_onceWhenOpen) {
            std::string error;
            std::string sdbPath = std::format("{}/sdb/dialogs/dialogsphrases.sdb", rootDirectory);
            if (!SDB_Parser::parse(sdbPath, m_sdbDialogs, &error))
                LogFmt("Load dialogsphrases.sdb error: {}", error);

            std::string varsPath = std::format("{}/scripts/dialogs_special/zlato_vars.scr", rootDirectory);
            if (!CsExecutor::readGlobalVariables(varsPath, m_globalVars, &error))
                LogFmt("Load zlato_vars.scr error: {}", error);
            m_onceWhenOpen = true;
        }

        bool isOpenedWindow = true;
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("CS Viewer", &showWindow);

        ImGui::BeginChild("left pane", ImVec2(340, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        m_textFilterFile.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(csFiles.size()); ++i)
            {
                if (m_textFilterFile.PassFilter(csFiles[i].c_str())
                    && ImGui::Selectable(csFiles[i].c_str(), m_selectedIndex == i))
                {
                    m_selectedIndex = i;

                    m_csError.clear();
                    m_csData.nodes.clear();
                    m_funcNodes.clear();

                    std::string csPath = std::format("{}/{}", rootDirectory, csFiles[i]);
                    CS_Parser::parse(csPath, m_csData, &m_csError);
                    // injectPlaySoundFunc(737, "kotar\\zdravstvuj_rasskazhi.ogg");
                    // CS_Parser::save(std::format("{}.new", csPath), m_csData, &m_csError);

                    // Заполнение данных для фильтрации функций
                    m_funcNodes.resize(m_csData.nodes.size(), false);
                    for (size_t i = 0; i < m_csData.nodes.size(); ++i) {
                        const auto& node = m_csData.nodes[i];
                        if (node.opcode == kFunc) {
                            m_funcNodes[i] = true;
                            for (int j = 0; j < node.args.size(); ++j) {
                                int32_t idx = node.args[j];
                                if (idx == -1) break;

                                m_funcNodes[++i] = true;
                            }
                        }
                    }

                    needResetScroll = true;
                    needUpdate = true;
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();


        ImGui::BeginChild("right pane");
        m_textFilterString.Draw();

            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (!m_csData.nodes.empty()) {
                char nodeInfoBuffer[4096];

                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                const ImGuiStyle& style = ImGui::GetStyle();
                ImGui::PushFont(NULL, style.FontSizeBase - 1.0f);
                for (size_t i = 0; i < m_csData.nodes.size(); ++i) {
                    const CS_Node* prevNode = nullptr;
                    if (i > 0) {
                        prevNode = &m_csData.nodes[i - 1];
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

                    const CS_Node& node = m_csData.nodes[i];
                    node.toStringBuffer(nodeInfoBuffer, (isDialogPhrase && m_showDialogPhrases), m_sdbDialogs.strings);

                    if (m_showOnlyFunctions && m_funcNodes[i]
                        || !m_showOnlyFunctions) {
                        if (m_textFilterString.PassFilter(nodeInfoBuffer)) {
                            bool isFunc = node.opcode == kFunc;
                            bool isExec = m_csExecutorViewer.isNodeExecuted(i);
                            ImVec4 textColor = isExec ? m_execTextColor
                                                      : (isFunc ? m_funcTextColor
                                                                : style.Colors[ImGuiCol_Text]);
                            ImGui::TextColored(textColor, "[i:%zu] %s", i, nodeInfoBuffer);
                        }
                    }
                }
                ImGui::PopFont();
            } else if (m_selectedIndex >= 0) {
                ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_csError.c_str());
            }
            ImGui::EndChild();

        ImGui::Checkbox("Funcs only", &m_showOnlyFunctions);
        ImGui::SameLine();
        ImGui::Checkbox("Dialog phrases", &m_showDialogPhrases);
        ImGui::SameLine();

        if (!m_csData.nodes.empty()) {
            ImGui::BeginDisabled(m_showExecuteWindow);
            if (ImGui::Button("Execute")) {
                m_showExecuteWindow = true;
            }
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        ImGui::Text("Executed: %d%%", m_csExecutorViewer.executedPercent());

        ImGui::EndChild();

        ImGui::End();

        m_csExecutorViewer.update(m_showExecuteWindow, needUpdate, m_csData.nodes, m_globalVars, m_sdbDialogs);
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_csError.clear();
        m_csData.nodes.clear();
        m_textFilterFile.Clear();
        m_textFilterString.Clear();
        m_sdbDialogs = {};
        m_globalVars.clear();
        m_funcNodes.clear();
        m_onceWhenOpen = false;
        m_onceWhenClose = true;
    }
}

void CsViewer::injectPlaySoundFunc(size_t insertPos, std::string_view soundFile)
{
    CS_Node assign;
    assign.opcode = kAssign;
    assign.a = insertPos + 1;
    assign.b = insertPos + 2;
    assign.c = assign.d = insertPos + 4;

    CS_Node strVar;
    strVar.opcode = kStringVarName;
    strVar.text = "result";

    CS_Node func;
    func.opcode = kFunc;
    func.value = kD_PlaySound;
    func.args[0] = insertPos + 3;

    CS_Node strLit;
    strLit.opcode = kStringLiteral;
    strLit.text = std::string(soundFile);

    std::array<CS_Node, 4> playSoundNodes = {assign, strVar, func, strLit};
    m_csData.insertNodes(insertPos, playSoundNodes);

    m_csData.nodes[insertPos - 4].c = m_csData.nodes[insertPos - 4].d = insertPos;
}
