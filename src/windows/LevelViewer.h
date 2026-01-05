#pragma once
#include "Level.h"

struct ImRect;

class LevelViewer {
public:
    LevelViewer();

    void update(bool& showWindow, std::string_view rootDirectory, Level& level);
    bool isAnimating(const Level& level) const;

private:
    void drawMenuBar(std::string_view rootDirectory, Level& level);
    void handleHotkeys(Level& level, bool anyWindowFocused);

    bool isVisibleInWindow(const ImRect& rect) const;
    bool leftMouseDownOnLevel(const Level& level) const;

    ImVec2 computeMinimapSize(const Level& level, bool hasMinimap);
    ImVec2 computeMinimapPosition(const Level& level, ImVec2 minimapSize);
    ImVec2 transformPoint(const ImVec2& pointInSource, const ImRect& sourceRect, const ImRect& targetRect);
    const char* maskSoundToString(MapDataSound sound);

    std::string personInfo(const SEF_Person& person) const;
    std::string_view personName(const Level& level, const SEF_Person& person) const;

    void levelScrollTo(Level& level, ImVec2 targetPos, ImVec2 targetSize);
    void handleLevelDragScroll(Level& level);
    void drawSelectionHighlight(Level& level, ImVec2 drawPosition);
    void drawMinimap(Level& level, const ImRect& levelRect, ImRect& minimapRect);
    void drawInfo(Level& level, const ImRect& levelRect, ImVec2 drawPosition);

    void drawMapTiles(Level& level, ImVec2 drawPosition);
    ImU32 getTileColor(const MapTile& tile, MapTilesMode mode);
    void drawTileBorderAndTooltip(const MapTile& tile, ImVec2 tileTopLeft, ImVec2 tileBottomRight, int tileColumn, int tileRow, int chunkColumn, int chunkRow, Level& level);
    void drawChunkBorder(ImVec2 chunkTopLeft, Level& level);

    void drawPersons(Level& level, ImVec2 drawPosition);
    void drawPointsEntrance(Level& level, ImVec2 drawPosition);
    void drawCellGroups(Level& level, ImVec2 drawPosition);
    void drawCellGroup(LevelImgui& imgui, ImVec2 drawPosition, const CellGroup& group, int groupIndex, SDL_Color color, bool drawConnectedLine);
    void drawAnimations(Level& level, ImVec2 drawPosition);
    void drawSounds(Level& level, ImVec2 drawPosition);
    void drawTriggers(Level& level, ImVec2 drawPosition);

    void drawObjectsList(Level& level);
};
