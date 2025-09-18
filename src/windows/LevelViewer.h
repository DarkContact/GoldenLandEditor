#pragma once
#include "Level.h"

struct ImRect;

class LevelViewer
{
public:
    LevelViewer() = delete;

    static bool update(bool& showWindow, Level& level);

private:
    static ImVec2 computeMinimapSize(const Level& level, bool hasMinimap);
    static ImVec2 computeMinimapPosition(const Level& level, ImVec2 minimapSize);

    static void updateMinimap(Level& level, ImRect& minimapRect);
    static void updateInfo(Level& level, ImVec2 drawPosition);
    static void drawMask(Level& level, ImVec2 drawPosition);
    static void drawPersons(Level& level, ImVec2 drawPosition);
};
