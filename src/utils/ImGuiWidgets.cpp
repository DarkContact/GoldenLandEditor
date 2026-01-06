#include "ImGuiWidgets.h"

#include <cassert>

#include "imgui.h"
#include "imgui_internal.h"

bool ImGuiWidgets::ComboBoxWithIndex(std::string_view label, const std::vector<std::string>& items, int& selectedIndex) {
    if (items.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size()))
        return false;

    const char* currentItem = items[selectedIndex].c_str();

    bool changed = false;
    if (ImGui::BeginCombo(label.data(), currentItem)) {
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

void ImGuiWidgets::Loader(std::string_view label, bool& showWindow)
{
    ImGuiIO& io = ImGui::GetIO();
    auto title = "Loading";

    if ( showWindow && !ImGui::IsPopupOpen(title) ) {
        ImGui::OpenPopup(title);
    }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(title, &showWindow, ImGuiWindowFlags_AlwaysAutoResize |
                                                   ImGuiWindowFlags_NoDecoration |
                                                   ImGuiWindowFlags_NoInputs |
                                                   ImGuiWindowFlags_NoNav))
    {
        ImGui::ProgressBar(-0.75f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), label.data());
        ImGui::EndPopup();
    }
}

void ImGuiWidgets::ShowMessageModal(std::string_view title, std::string& message)
{
    assert(!title.empty());

    ImGuiIO& io = ImGui::GetIO();
    static bool isShow = false;

    if (!isShow && !message.empty()) {
        ImGui::OpenPopup(title.data());
        isShow = true;
    }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, 0.0f), ImGuiCond_Always);
    if (ImGui::BeginPopupModal(title.data())) {
        ImGui::TextWrapped("%s", message.c_str());

        // Центрируем кнопку "OK"
        float buttonWidth = 60.0f;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float offset = (availableWidth - buttonWidth) * 0.5f;
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
            message.clear();
            isShow = false;
        }

        ImGui::EndPopup();
    }
}

bool ImGuiWidgets::ShowMessageModalEx(std::string_view title, const std::function<void()>& callback)
{
    assert(!title.empty());
    assert(callback);

    ImGuiIO& io = ImGui::GetIO();
    static bool isShow = false;

    if (!isShow) {
        ImGui::OpenPopup(title.data());
        isShow = true;
    }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(title.data(), &isShow, ImGuiWindowFlags_AlwaysAutoResize)) {
        callback();

        // Центрируем кнопку "OK"
        float buttonWidth = 60.0f;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float offset = (availableWidth - buttonWidth) * 0.5f;
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
            isShow = false;
        }

        ImGui::EndPopup();
    }

    return isShow;
}

void SetTooltipVStacked(const char* fmt, va_list args) {
    if (!ImGui::BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None))
        return;
    ImGui::TextV(fmt, args);
    ImGui::EndTooltip();
}

void ImGuiWidgets::SetTooltipStacked(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SetTooltipVStacked(fmt, args);
    va_end(args);
}

bool ImGuiWidgets::BeginTooltipStacked()
{
    return ImGui::BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None);
}

void ImGuiWidgets::EndTooltipStacked()
{
    ImGui::EndTooltip();
}
