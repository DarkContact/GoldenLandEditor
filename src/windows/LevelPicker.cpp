#include "LevelPicker.h"

#include "imgui.h"
#include "ImguiHelper.h"

bool LevelPicker::update(bool& showWindow, const std::vector<std::string>& levelNames, int& selectedLevelIndex)
{
    bool pressLoadButton = false;

    ImGui::Begin("Level picker", &showWindow);

    if (!levelNames.empty()) {
        if (ImguiHelper::ComboBoxWithIndex("Levels", levelNames, selectedLevelIndex)) {

        }

        if (ImGui::Button("Load")) {
            showWindow = false;
            pressLoadButton = true;
        }
    }

    ImGui::End();

    return pressLoadButton;
}
