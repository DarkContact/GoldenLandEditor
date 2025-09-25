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
    static ImVec2 transformPoint(const ImVec2& pointInSource, const ImRect& sourceRect, const ImRect& targetRect);

    static void handleLevelDragScroll(Level& level);
    static void drawMinimap(Level& level, const ImRect& levelRect, ImRect& minimapRect);
    static void drawInfo(Level& level, const ImRect& levelRect, ImVec2 drawPosition);
    static void drawMask(Level& level, ImVec2 drawPosition);
    static void drawPersons(Level& level, ImVec2 drawPosition);
    static void drawPointsEntrance(Level& level, ImVec2 drawPosition);
    static void drawCellGroups(Level& level, ImVec2 drawPosition);

    static std::string maskSoundToString(MaskSound sound);

    static std::string personInfo(const SEF_Person& person);
};
