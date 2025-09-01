#pragma once
#include "Level.h"

class LevelViewer
{
public:
    LevelViewer() = delete;

    static bool update(bool& showWindow, Level& level);

private:
    static void updateMinimap(Level& level);
    static void updateInfo(Level& level);
    static void drawMask(Level& level);
};

