#include "CsExecutorViewer.h"

#include <memory>
#include <format>

#include "imgui.h"

#include "CsExecutor.h"

void CsExecutorViewer::update(bool& showWindow,
                              bool& needUpdate,
                              std::span<const CS_Node> nodes,
                              const UMapStringVar_t& globalVars)
{
    static std::unique_ptr<CsExecutor> pExecutor = nullptr;

    if (needUpdate) {
        pExecutor = std::make_unique<CsExecutor>(nodes, globalVars);
        needUpdate = false;
    }

    if (!pExecutor)
        return;

    if (showWindow) {
        ImGui::SetNextWindowSize({600, 400}, ImGuiCond_FirstUseEver);
        ImGui::Begin("CS Executor", &showWindow, ImGuiWindowFlags_HorizontalScrollbar);

        int nodeIndex = pExecutor->currentNodeIndex();

        ImGui::Text("[i:%d] %s (step:%d)",
                    nodeIndex,
                    nodes[nodeIndex].toString(false).c_str(),
                    pExecutor->counter());
        ImGui::Text("%s", std::format("vars: {}", pExecutor->variablesInfo()).c_str());
        ImGui::Text("%s", std::format("funcs: {}", pExecutor->funcsInfo()).c_str());

        if (ImGui::Button("Step")) {
            pExecutor->next();
        }
        ImGui::SameLine();
        if (ImGui::Button("Execute all")) {
            while(pExecutor->next());
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            pExecutor->restart();
        }

        ImGui::End();
    }
}
