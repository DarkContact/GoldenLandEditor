#include "CsExecutorViewer.h"

#include <memory>
#include <format>

#include "imgui.h"

#include "CsExecutor.h"
#include "CsViewer.h"

#include "parsers/SDB_Parser.h"

void CsExecutorViewer::update(bool& showWindow, bool& needUpdate, std::string_view rootDirectory, std::span<const CS_Node> nodes)
{
    static std::unique_ptr<CsExecutor> pExecutor = nullptr;
    static std::string uiError;

    if (needUpdate) {
        pExecutor = std::make_unique<CsExecutor>(nodes);
        pExecutor->readGlobalVariables(rootDirectory, &uiError);
        pExecutor->readScriptVariables(&uiError);
        needUpdate = false;
    }

    if (!pExecutor)
        return;

    if (showWindow) {
        ImGui::SetNextWindowSize({600, 400}, ImGuiCond_FirstUseEver);
        ImGui::Begin("CS Executor", &showWindow, ImGuiWindowFlags_HorizontalScrollbar);

        ImGui::Text("[i:%d] %s (step:%d)",
                    pExecutor->currentNodeIndex(),
                    CsViewer::csNodeString(nodes[pExecutor->currentNodeIndex()], {}, false).c_str(),
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
