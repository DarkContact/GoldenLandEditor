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

void ImguiHelper::Loader(const char* label, bool& showWindow)
{
    ImGuiIO& io = ImGui::GetIO();

    if (showWindow)
        ImGui::OpenPopup("Loading");

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Loading", &showWindow, ImGuiWindowFlags_AlwaysAutoResize |
                                                           ImGuiWindowFlags_NoDecoration |
                                                           ImGuiWindowFlags_NoInputs |
                                                           ImGuiWindowFlags_NoNav)) {
        ImGui::ProgressBar(-0.75f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), label);
        ImGui::EndPopup();
    }
}
