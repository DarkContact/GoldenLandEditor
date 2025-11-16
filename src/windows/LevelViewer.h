#pragma once
#include "Level.h"

struct ImRect;

class LevelViewer
{
public:
    LevelViewer() = delete;

    static bool update(bool& showWindow, std::string_view rootDirectory, Level& level);

private:
    static ImVec2 computeMinimapSize(const Level& level, bool hasMinimap);
    static ImVec2 computeMinimapPosition(const Level& level, ImVec2 minimapSize);
    static ImVec2 transformPoint(const ImVec2& pointInSource, const ImRect& sourceRect, const ImRect& targetRect);
    static bool leftMouseDownOnLevel(const Level& level);
    static const char* maskSoundToString(MapDataSound sound);
    static std::string personInfo(const SEF_Person& person);

    static void handleLevelDragScroll(Level& level);
    static void drawMinimap(Level& level, const ImRect& levelRect, ImRect& minimapRect);
    static void drawInfo(Level& level, const ImRect& levelRect, ImVec2 drawPosition);

    static void drawMapTiles(Level& level, ImVec2 drawPosition);
    static ImU32 getTileColor(const MapTile& tile, MapTilesMode mode);
    static void drawTileBg(const MapTile& tile, ImVec2 tileTopLeft, ImVec2 tileBottomRight, Level& level);
    static void drawTileBorderAndTooltip(const MapTile& tile, ImVec2 tileTopLeft, ImVec2 tileBottomRight, int tileColumn, int tileRow, int chunkColumn, int chunkRow, Level& level);
    static void drawChunkBorder(ImVec2 chunkTopLeft, Level& level);

    static void drawPersons(Level& level, ImVec2 drawPosition);
    static void drawPointsEntrance(Level& level, ImVec2 drawPosition);
    static void drawCellGroups(Level& level, ImVec2 drawPosition);
    static void drawCellGroup(LevelImgui& imgui, ImVec2 drawPosition, const CellGroup& group, int groupIndex, SDL_Color color, bool drawConnectedLine);
    static void drawAnimations(Level& level, ImVec2 drawPosition);
    static void drawSounds(Level& level, ImVec2 drawPosition);
};
