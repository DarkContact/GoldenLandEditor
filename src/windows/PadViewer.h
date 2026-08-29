#pragma once
#include <vector>
#include <string>

#include "imgui.h"
#include "parsers/PAD_Parser.h"

struct SDL_Renderer;

class PadViewer
{
public:
    PadViewer();

    void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& padFiles);
    bool isAnimating() const;

private:
    int m_selectedIndex = -1;
    bool m_onceWhenClose = true;
    ImGuiTextFilter m_textFilter;
    std::string m_error;
    std::optional<PAD_Data> m_padData;
};
