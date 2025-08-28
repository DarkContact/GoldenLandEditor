#include "LevelPicker.h"

#include <vector>
#include <string>

#include "imgui.h"
#include "ImguiHelper.h"

void LevelPicker::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& levelNames, Level& level)
{
    static int selectedLevelIndex = 0;

    ImGui::Begin("Level picker", &showWindow);

    if (!levelNames.empty()) {
        if (ImguiHelper::ComboBoxWithIndex("Levels", levelNames, selectedLevelIndex)) {

        }

        if (ImGui::Button("Load")) {
            level.loadLevel(rootDirectory, levelNames[selectedLevelIndex]);
            showWindow = false;
        }
    }

    ImGui::End();
}
