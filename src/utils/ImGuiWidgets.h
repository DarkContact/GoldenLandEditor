#pragma once
#include <vector>
#include <string>

class ImGuiWidgets
{
public:
    ImGuiWidgets() = delete;

    static bool ComboBoxWithIndex(const char* label, const std::vector<std::string>& items, int& selectedIndex);
    static void Loader(const char* label, bool& showWindow);
};

