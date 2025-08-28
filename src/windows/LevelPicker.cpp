#include "LevelPicker.h"

#include <vector>
#include <string>

#include "imgui.h"
#include "ImguiHelper.h"

void LevelPicker::update(bool& showWindow, const std::vector<std::string>& levelNames)
{
    static int selectedLevelIndex = 0;

    ImGui::Begin("Level picker", &showWindow);

    if (!levelNames.empty()) {
        if (ImguiHelper::ComboBoxWithIndex("Levels", levelNames, selectedLevelIndex)) {

        }

        if (ImGui::Button("Load")) {

            showWindow = false;
        }
    }

    ImGui::End();
}
