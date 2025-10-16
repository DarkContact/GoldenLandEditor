#pragma once
#include <string_view>
#include <vector>
#include <string>

class ImGuiWidgets
{
public:
    ImGuiWidgets() = delete;

    static bool ComboBoxWithIndex(std::string_view label, const std::vector<std::string>& items, int& selectedIndex);
    static void Loader(std::string_view label, bool& showWindow);
    static void ShowMessageModal(std::string_view title, std::string& message);

};

