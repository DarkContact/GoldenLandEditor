#pragma once
#include <vector>
#include <string>

#include "imgui.h"

#include "graphics/Texture.h"

struct SDL_Renderer;

class CsxViewer {
public:
    CsxViewer();

    void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles);

private:
    struct SaveDialogData {
        std::string csxPath;
        SDL_Renderer* renderer;
    };

    int m_selectedIndex = -1;
    std::vector<Texture> m_csxTextures;
    std::string m_csxTextureError;
    ImVec4 m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    int m_activeButtonIndex = 0;
    ImGuiTextFilter m_textFilter;
    SaveDialogData m_saveDialogData;
    bool m_onceWhenClose = true;
};


