#include "CsExecutorViewer.h"

#include <algorithm>
#include <format>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "utils/StringUtils.h"
#include "utils/Formatters.h"

CsExecutorViewer::CsExecutorViewer() {}

void CsExecutorViewer::update(bool& showWindow,
                              bool& needUpdate,
                              std::string_view title,
                              std::span<const CS_Node> nodes,
                              const std::map<int, std::string>& dialogsPhrases,
                              const StringHashTable<AgeVariable_t>& globalVars)
{
    if (needUpdate) {
        m_pExecutor = std::make_unique<CsExecutor>(nodes, globalVars);
        m_isDialog = false;
        needUpdate = false;
    }

    if (showWindow && m_pExecutor && !nodes.empty()) {
        ImGui::SetNextWindowSize({600, 500}, ImGuiCond_FirstUseEver);
        char titleBuffer[512];
        StringUtils::formatToBuffer(titleBuffer, "{}###CsExecutorViewer", title);
        if (ImGui::Begin(titleBuffer, &showWindow, ImGuiWindowFlags_HorizontalScrollbar)) {

            if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& scriptVars = m_pExecutor->scriptVars();
                for (auto& [name, value] : scriptVars) {
                    std::visit([&name] (auto& v) {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            ImGui::InputText(name.c_str(), &v);
                        } else {
                            ImGuiDataType type;
                            if constexpr (std::is_same_v<T, int32_t>)
                                type = ImGuiDataType_S32;
                            else if constexpr (std::is_same_v<T, uint32_t>)
                                type = ImGuiDataType_U32;
                            else if constexpr (std::is_same_v<T, double>)
                                type = ImGuiDataType_Double;

                            ImGui::InputScalar(name.c_str(), type, &v);
                        }
                    }, value);
                }
            }
            ImGui::Separator();

            int nodeIndex = m_pExecutor->currentNodeIndex();
            char nodeInfoBuffer[4096];
            nodes[nodeIndex].toStringBuffer(nodeInfoBuffer, false);

            ImGui::Text("[i:%d] %s (step: %d) (status: %s)",
                        nodeIndex,
                        nodeInfoBuffer,
                        m_pExecutor->counter(),
                        m_pExecutor->currentStatusString());

            auto varsInfo = m_pExecutor->variablesInfo();
            std::sort(varsInfo.begin(), varsInfo.end());
            ImGui::TextWrapped("%s", std::format("vars: {}", varsInfo).c_str());

            auto funcsInfo = m_pExecutor->funcsInfo();
            ImGui::TextWrapped("%s", std::format("funcs: {}", funcsInfo).c_str());

            bool disabled = m_pExecutor->currentStatus() == CsExecutor::kEnd
                            || m_pExecutor->currentStatus() == CsExecutor::kInfinity
                            || m_pExecutor->currentStatus() == CsExecutor::kWaitUser;
            ImGui::BeginDisabled(disabled);

            bool dialogKeyOnce = !disabled
                                 && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_D, false)
                                 && ImGui::IsWindowFocused();
            if (ImGui::Button("Dialog") || dialogKeyOnce) {
                m_isDialog = true;
                while(m_pExecutor->next());
            }
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
                m_pExecutor->next();
            }
            ImGui::SameLine();
            if (ImGui::Button("Execute all")) {
                while(m_pExecutor->next());
            }

            ImGui::EndDisabled();

            ImGui::SameLine(0.0f, 36.0f);
            bool restartKeyOnce = ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_R, false)
                                  && ImGui::IsWindowFocused();
            if (ImGui::Button("Restart") || restartKeyOnce) {
                m_pExecutor->restart(m_resetAllVariables);
                m_isDialog = false;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Reset all variables", &m_resetAllVariables);

            if (m_pExecutor->currentStatus() == CsExecutor::kWaitUser) {
                auto dialogData = m_pExecutor->dialogsData();
                if (dialogData[0] != -1) {
                    ImGui::SeparatorText("Dialog");

                    std::string_view sayPhrase = "[NOT FOUND!]";
                    if (auto it = dialogsPhrases.find(dialogData[0]); it != dialogsPhrases.cend()) {
                        sayPhrase = it->second;
                    }
                    ImGui::TextWrapped("%s", sayPhrase.data());
                    for (int i = 1; i < 11; ++i) {
                        if (dialogData[i] == -1) break;

                        char indexStr[3];
                        StringUtils::formatToBuffer(indexStr, "{}", i);

                        ImGuiKey key{ImGuiKey::ImGuiKey_0 + i}; // [ImGuiKey_1 ... ImGuiKey_9]
                        if (i == 10) {
                            key = ImGuiKey::ImGuiKey_0;
                        }

                        bool buttonKeyOnce = ImGui::IsKeyPressed(key, false)
                                             && ImGui::IsWindowFocused();
                        if (ImGui::Button(indexStr) || buttonKeyOnce) {
                            m_pExecutor->userInput(i);
                            if (m_isDialog) {
                                while(m_pExecutor->next());
                            }
                        }
                        ImGui::SameLine();
                        std::string_view answerPhrase = "[NOT FOUND!]";
                        if (auto it = dialogsPhrases.find(dialogData[i]); it != dialogsPhrases.cend()) {
                            answerPhrase = it->second;
                        }
                        ImGui::TextWrapped("%s", answerPhrase.data());
                    }
                }
            }
        }
        ImGui::End();
    }
}

bool CsExecutorViewer::isNodeExecuted(int index) const
{
    if (m_pExecutor) {
        return m_pExecutor->isNodeExecuted(index);
    }
    return false;
}

float CsExecutorViewer::executedPercent() const
{
    if (m_pExecutor) {
        return m_pExecutor->executedPercent();
    }
    return 0;
}
