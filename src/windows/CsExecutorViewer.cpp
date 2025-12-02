#include "CsExecutorViewer.h"

#include <algorithm>
#include <format>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

CsExecutorViewer::CsExecutorViewer() {

}

void CsExecutorViewer::update(bool& showWindow,
                              bool& needUpdate,
                              std::span<const CS_Node> nodes,
                              const UMapStringVar_t& globalVars)
{
    if (needUpdate) {
        m_pExecutor = std::make_unique<CsExecutor>(nodes, globalVars);
        needUpdate = false;
    }

    if (showWindow && m_pExecutor) {
        ImGui::SetNextWindowSize({600, 400}, ImGuiCond_FirstUseEver);
        ImGui::Begin("CS Executor", &showWindow, ImGuiWindowFlags_HorizontalScrollbar);

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

        int nodeIndex = m_pExecutor->currentNodeIndex();
        char nodeInfoBuffer[4096];
        nodes[nodeIndex].toStringBuffer(nodeInfoBuffer, false);

        ImGui::Text("[i:%d] %s (step:%d)",
                    nodeIndex,
                    nodeInfoBuffer,
                    m_pExecutor->counter());

        auto varsInfo = m_pExecutor->variablesInfo();
        std::sort(varsInfo.begin(), varsInfo.end());
        ImGui::Text("%s", std::format("vars: {}", varsInfo).c_str());

        auto funcsInfo = m_pExecutor->funcsInfo();
        ImGui::Text("%s", std::format("funcs: {}", funcsInfo).c_str());

        if (ImGui::Button("Step")) {
            m_pExecutor->next();
        }
        ImGui::SameLine();
        if (ImGui::Button("Execute all")) {
            while(m_pExecutor->next());
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            m_pExecutor->restart();
        }

        ImGui::End();
    }
}
