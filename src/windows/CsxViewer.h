#pragma once
#include <vector>
#include <string>

struct SDL_Renderer;

class CsxViewer
{
public:
    CsxViewer() = delete;

    static void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& csxFiles);
};

