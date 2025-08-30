#pragma once
#include <vector>
#include <string>

#include "SDL3/SDL_render.h"

class CsxViewer
{
public:
    CsxViewer() = delete;

    static bool update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles);
};

