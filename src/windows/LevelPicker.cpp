#include "LevelPicker.h"

#include "imgui.h"
#include "utils/ImGuiWidgets.h"

bool LevelPicker::update(bool& showWindow, const std::vector<std::string>& levelNames, int& selectedLevelIndex)
{
    bool pressLoadButton = false;

    if (showWindow)
        ImGui::OpenPopup("Level picker"); // FIXME: Вызывать 1 раз

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Level picker", &showWindow, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGuiWidgets::ComboBoxWithIndex("Levels", levelNames, selectedLevelIndex);

        if (ImGui::Button("Load")) {
            showWindow = false;
            pressLoadButton = true;
        }

        ImGui::EndPopup();
    }

    return pressLoadButton;
}
