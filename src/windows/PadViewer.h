#pragma once
#include <unordered_map>
#include <vector>
#include <string>

#include "imgui.h"
#include "parsers/PAD_Parser.h"

class Texture;
struct SDL_Renderer;

class PadViewer {
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
    int m_selectedAnimationIndex = -1;
    ImVec4 m_bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    std::unordered_map<PAD_AnimationTypeMask, std::pair<Texture, Texture>> m_animationTextures;
};
