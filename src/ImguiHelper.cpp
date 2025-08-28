#include "ImguiHelper.h"

#include "imgui.h"

bool ImguiHelper::ComboBoxWithIndex(const char* label, const std::vector<std::string>& items, int& selectedIndex) {
    if (items.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size()))
        return false;

    const char* currentItem = items[selectedIndex].c_str();

    bool changed = false;
    if (ImGui::BeginCombo(label, currentItem)) {
        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            bool isSelected = (i == selectedIndex);
            if (ImGui::Selectable(items[i].c_str(), isSelected)) {
                selectedIndex = i;
                changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}
