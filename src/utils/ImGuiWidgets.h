#pragma once
#include <string_view>
#include <functional>
#include <vector>
#include <string>

#include "imgui.h"

class ImGuiWidgets
{
public:
    ImGuiWidgets() = delete;

    static bool ComboBoxWithIndex(std::string_view label, const std::vector<std::string>& items, int& selectedIndex);
    static void Loader(std::string_view label, bool& showWindow);
    static void ShowMessageModal(std::string_view title, std::string& message);
    static bool ShowMessageModalEx(std::string_view title, const std::function<void()>& callback);

    static void SetTooltipStacked(const char* fmt, ...) IM_FMTARGS(1);
};
