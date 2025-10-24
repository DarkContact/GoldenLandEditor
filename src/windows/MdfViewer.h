#pragma once
#include <vector>
#include <string>

struct SDL_Renderer;

class MdfViewer
{
public:
    MdfViewer() = delete;

    static void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& mdfFiles);
};

