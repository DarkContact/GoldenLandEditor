#include "CsExecutorViewer.h"

#include <algorithm>
#include <format>

#include "imgui.h"

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
