#include "LevelViewer.h"

#include <format>

#include "SDL3/SDL_timer.h"
#include "imgui_internal.h"
#include "imgui.h"

#include "utils/TracyProfiler.h"

bool LevelViewer::update(bool& showWindow, Level& level)
{
    Tracy_ZoneScopedN("LevelViewer::update");
    Tracy_ZoneText(level.data().name.c_str(), level.data().name.size());
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(level.data().name.c_str(), &showWindow, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Minimap", "Tab", level.data().imgui.showMinimap)) {
                level.data().imgui.showMinimap = !level.data().imgui.showMinimap;
            }
            if (ImGui::MenuItem("Info", "I", level.data().imgui.showMetaInfo)) {
                level.data().imgui.showMetaInfo = !level.data().imgui.showMetaInfo;
            }
            if (ImGui::MenuItem("Tiles", "T", level.data().imgui.showMapTiles)) {
                level.data().imgui.showMapTiles = !level.data().imgui.showMapTiles;
            }
            if (ImGui::MenuItem("Persons", "P", level.data().imgui.showPersons)) {
                level.data().imgui.showPersons = !level.data().imgui.showPersons;
            }
            if (ImGui::MenuItem("Entrance points", "E", level.data().imgui.showEntrancePoints)) {
                level.data().imgui.showEntrancePoints = !level.data().imgui.showEntrancePoints;
            }
            if (ImGui::MenuItem("Cells groups", "C", level.data().imgui.showCellGroups)) {
                level.data().imgui.showCellGroups = !level.data().imgui.showCellGroups;
            }
            if (ImGui::MenuItem("Animations", "A", level.data().imgui.showAnimations)) {
                level.data().imgui.showAnimations = !level.data().imgui.showAnimations;
            }
            ImGui::EndMenu();
        }

        if (level.data().imgui.showMapTiles && ImGui::BeginMenu("Tiles mode")) {
            if (ImGui::MenuItem("Relief", "Ctrl + 1", level.data().imgui.mapTilesMode == MapTilesMode::Relief)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Relief;
            }
            if (ImGui::MenuItem("Sound", "Ctrl + 2", level.data().imgui.mapTilesMode == MapTilesMode::Sound)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Sound;
            }
            if (ImGui::MenuItem("Mask", "Ctrl + 3", level.data().imgui.mapTilesMode == MapTilesMode::Mask)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Mask;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }


    if (ImGui::IsWindowFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Tab, false)) {
            level.data().imgui.showMinimap = !level.data().imgui.showMinimap;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_I, false)) {
            level.data().imgui.showMetaInfo = !level.data().imgui.showMetaInfo;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_T, false)) {
            level.data().imgui.showMapTiles = !level.data().imgui.showMapTiles;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_P, false)) {
            level.data().imgui.showPersons = !level.data().imgui.showPersons;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_E, false)) {
            level.data().imgui.showEntrancePoints = !level.data().imgui.showEntrancePoints;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_C, false)) {
            level.data().imgui.showCellGroups = !level.data().imgui.showCellGroups;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_A, false)) {
            level.data().imgui.showAnimations = !level.data().imgui.showAnimations;
        }

        if (level.data().imgui.showMapTiles) {
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_1, false)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Relief;
            }
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_2, false)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Sound;
            }
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_3, false)) {
                level.data().imgui.mapTilesMode = MapTilesMode::Mask;
            }
        }
    }

    // Отрисовка уровня
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)level.data().background.get(), ImVec2(level.data().background->w, level.data().background->h));

    if (level.data().imgui.showAnimations)
    {
        drawAnimations(level, startPos);
    }

    handleLevelDragScroll(level);

    // Отрисовка персонажей
    if (level.data().imgui.showPersons)
    {
        drawPersons(level, startPos);
    }

    if (level.data().imgui.showEntrancePoints)
    {
        drawPointsEntrance(level, startPos);
    }

    if (level.data().imgui.showCellGroups)
    {
        drawCellGroups(level, startPos);
    }

    if (level.data().imgui.showMapTiles)
    {
        drawMapTiles(level, startPos);
    }

    ImRect minimapRect;
    const ImRect levelRect(startPos, {startPos.x + level.data().background->w,
                                      startPos.y + level.data().background->h});
    if (level.data().imgui.showMinimap) {
        drawMinimap(level, levelRect, minimapRect);
        minimapRect.Max.y += 16.0f;
    } else {
        ImVec2 minimapSize = {200.0f, 0.0f};
        ImVec2 minimapPosition = computeMinimapPosition(level, minimapSize);

        minimapRect = {minimapPosition, {minimapPosition.x + minimapSize.x,
                                         minimapPosition.y + minimapSize.y + 8.0f}};
        level.data().imgui.minimapHovered = false;
    }

    if (level.data().imgui.showMetaInfo)
    {
        drawInfo(level, levelRect, {minimapRect.GetBL().x, minimapRect.GetBL().y});
    }

    ImGui::End();
    return true;
}


ImVec2 LevelViewer::computeMinimapSize(const Level& level, bool hasMinimap) {
    if (hasMinimap) {
        return ImVec2(level.data().minimap->w,
                      level.data().minimap->h);
    } else {
        const float scale = 200.0f / level.data().background->w;
        return ImVec2(scale * level.data().background->w,
                      scale * level.data().background->h);
    }
}

ImVec2 LevelViewer::computeMinimapPosition(const Level& level, ImVec2 minimapSize)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    const float titleBarHeight = ImGui::GetCurrentWindow()->TitleBarHeight;
    const float menuBarHeight = ImGui::GetCurrentWindow()->MenuBarHeight;
    const float scrollbarWidth = ImGui::GetCurrentWindow()->ScrollbarY ? (style.ScrollbarSize + style.ScrollbarPadding) : 0.0f;

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const float offset = 4.0f;
    const float borderOffset = 1.0f;

    return ImVec2(windowPos.x + windowSize.x - minimapSize.x - scrollbarWidth - offset - borderOffset,
                  windowPos.y + titleBarHeight + menuBarHeight + offset);
}

ImVec2 LevelViewer::transformPoint(const ImVec2& pointInSource, const ImRect& sourceRect, const ImRect& targetRect)
{
    ImVec2 normalized;
    normalized.x = (pointInSource.x - sourceRect.Min.x) / (sourceRect.Max.x - sourceRect.Min.x);
    normalized.y = (pointInSource.y - sourceRect.Min.y) / (sourceRect.Max.y - sourceRect.Min.y);

    ImVec2 pointInTarget;
    pointInTarget.x = targetRect.Min.x + normalized.x * (targetRect.Max.x - targetRect.Min.x);
    pointInTarget.y = targetRect.Min.y + normalized.y * (targetRect.Max.y - targetRect.Min.y);

    return pointInTarget;
}

// TODO: Добавить проверку что курсор мыши в активной области уровня
void LevelViewer::handleLevelDragScroll(Level& level) {
    Tracy_ZoneScoped;
    ImGuiIO& io = ImGui::GetIO();
    auto& imgui = level.data().imgui;
    if (!ImGui::IsWindowFocused()) {
        imgui.draggingLevel = false;
        return;
    }

    const bool isShiftLMB = io.KeyShift && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    const bool isMMB = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);

    // Начало перетаскивания по уровню
    if (!imgui.draggingLevel && (isShiftLMB || isMMB)) {
        imgui.draggingLevel = true;
        imgui.dragStartPos = io.MousePos;
        imgui.scrollStart = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
    }

    // Завершение перетаскивания
    if (imgui.draggingLevel &&
        !(ImGui::IsMouseDown(ImGuiMouseButton_Middle) || (io.KeyShift && ImGui::IsMouseDown(ImGuiMouseButton_Left)))) {
        imgui.draggingLevel = false;
    }

    // Обработка перемещения
    if (imgui.draggingLevel) {
        ImVec2 delta = ImVec2(io.MousePos.x - imgui.dragStartPos.x,
                              io.MousePos.y - imgui.dragStartPos.y);

        float newScrollX = imgui.scrollStart.x - delta.x;
        float newScrollY = imgui.scrollStart.y - delta.y;

        float maxScrollX = (float)level.data().background->w;
        float maxScrollY = (float)level.data().background->h;

        newScrollX = ImClamp(newScrollX, 0.0f, ImMax(0.0f, maxScrollX));
        newScrollY = ImClamp(newScrollY, 0.0f, ImMax(0.0f, maxScrollY));

        ImGui::SetScrollX(newScrollX);
        ImGui::SetScrollY(newScrollY);
    }
}

void LevelViewer::drawMinimap(Level& level, const ImRect& levelRect, ImRect& minimapRect)
{
    Tracy_ZoneScoped;
    bool hasMinimap = level.data().minimap.isValid();

    const ImVec2 minimapSize = computeMinimapSize(level, hasMinimap);
    const ImVec2 minimapPosition = computeMinimapPosition(level, minimapSize);
    minimapRect = ImRect(minimapPosition, ImVec2(minimapPosition.x + minimapSize.x, minimapPosition.y + minimapSize.y));
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Миникарта и белая обводка
    {
        ImGui::SetCursorScreenPos(minimapPosition);

        ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

        ImGui::ImageWithBg((ImTextureID)(hasMinimap ? level.data().minimap.get()
                                                    : level.data().background.get()),
                           minimapSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 1));

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        // Отрисовка персонажей на миникарте
        if (level.data().imgui.showPersons) {
            for (const SEF_Person& person : level.data().sefData.persons) {
                ImVec2 position(levelRect.Min.x + person.position.x * Level::tileWidth,
                                levelRect.Min.y + person.position.y * Level::tileHeight);

                ImVec2 minimapPosition = transformPoint(position, levelRect, minimapRect);
                drawList->AddCircleFilled(minimapPosition, 2.0f, IM_COL32(255, 228, 0, 220));
                drawList->AddCircle(minimapPosition, 3.0f, IM_COL32(0, 0, 0, 128));
            }
        }
    }

    // Рисуем рамку области видимости и обрабатываем drag / клик
    {
        float mapWidth = (float)level.data().background->w;
        float mapHeight = (float)level.data().background->h;

        ImVec2 contentMin = ImGui::GetCurrentWindow()->ContentRegionRect.Min;
        ImVec2 contentMax = ImGui::GetCurrentWindow()->ContentRegionRect.Max;
        ImVec2 viewSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

        float scaleX = minimapSize.x / mapWidth;
        float scaleY = minimapSize.y / mapHeight;

        ImVec2 viewTopLeft = ImVec2(minimapPosition.x + ImGui::GetScrollX() * scaleX,
                                    minimapPosition.y + ImGui::GetScrollY() * scaleY);
        ImVec2 viewBottomRight = ImVec2(viewTopLeft.x + viewSize.x * scaleX,
                                        viewTopLeft.y + viewSize.y * scaleY);

        ImGuiIO& io = ImGui::GetIO();
        ImRect viewRect(viewTopLeft, viewBottomRight);

        auto& imgui = level.data().imgui;

        bool hoveredView = viewRect.Contains(io.MousePos);
        bool hoveredMinimap = minimapRect.Contains(io.MousePos);
        level.data().imgui.minimapHovered = hoveredMinimap;

        // Начало перетаскивания рамки
        if (!imgui.draggingMinimap && hoveredView && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            imgui.draggingMinimap = true;
            imgui.minimapAnimating = false; // Останавливаем анимацию
            imgui.dragOffset = ImVec2(io.MousePos.x - viewTopLeft.x, io.MousePos.y - viewTopLeft.y);
        }

        // Завершение перетаскивания
        if (imgui.draggingMinimap && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            imgui.draggingMinimap = false;
        }

        // Обработка перемещения при drag
        if (imgui.draggingMinimap) {
            ImVec2 newTopLeft = ImVec2(io.MousePos.x - imgui.dragOffset.x, io.MousePos.y - imgui.dragOffset.y);

            float newScrollX = (newTopLeft.x - minimapPosition.x) / scaleX;
            float newScrollY = (newTopLeft.y - minimapPosition.y) / scaleY;

            newScrollX = ImClamp(newScrollX, 0.0f, mapWidth - viewSize.x);
            newScrollY = ImClamp(newScrollY, 0.0f, mapHeight - viewSize.y);

            ImGui::SetScrollX(newScrollX);
            ImGui::SetScrollY(newScrollY);
        }
        // Одиночный клик по миникарте (вне рамки)
        else if (!imgui.draggingMinimap && hoveredMinimap && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 clickPos = ImVec2(io.MousePos.x - minimapPosition.x,
                                     io.MousePos.y - minimapPosition.y);

            float centerX = clickPos.x / scaleX - viewSize.x * 0.5f;
            float centerY = clickPos.y / scaleY - viewSize.y * 0.5f;

            centerX = ImClamp(centerX, 0.0f, mapWidth - viewSize.x);
            centerY = ImClamp(centerY, 0.0f, mapHeight - viewSize.y);

            imgui.minimapScrollStart = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            imgui.minimapScrollTarget = ImVec2(centerX, centerY);
            imgui.minimapAnimTime = 0.0f;
            imgui.minimapAnimating = true;
        }

        // Плавное перемещение scroll при клике
        if (imgui.minimapAnimating) {
            imgui.minimapAnimTime += io.DeltaTime * 6.0f;
            if (imgui.minimapAnimTime >= 1.0f) {
                imgui.minimapAnimTime = 1.0f;
                imgui.minimapAnimating = false;
            }

            ImVec2 scroll = ImLerp(imgui.minimapScrollStart, imgui.minimapScrollTarget, imgui.minimapAnimTime);
            ImGui::SetScrollX(scroll.x);
            ImGui::SetScrollY(scroll.y);
        }

        // Обновим рамку
        viewTopLeft = ImVec2(minimapPosition.x + ImGui::GetScrollX() * scaleX,
                             minimapPosition.y + ImGui::GetScrollY() * scaleY);
        viewBottomRight = ImVec2(viewTopLeft.x + viewSize.x * scaleX,
                                 viewTopLeft.y + viewSize.y * scaleY);

        // Рисуем рамку
        {
            ImVec2 size = ImVec2(viewBottomRight.x - viewTopLeft.x, viewBottomRight.y - viewTopLeft.y);
            float gapW = size.x * 0.25f;
            float gapH = size.y * 0.25f;

            const float thickness = 1.0f;
            const ImU32 opaque = IM_COL32(255, 255, 0, 255);
            const ImU32 transparent = IM_COL32(255, 255, 0, 48);

            float cornerLengthX = (size.x - gapW) * 0.5f;
            float cornerLengthY = (size.y - gapH) * 0.5f;

            // Top-left
            drawList->AddRectFilledMultiColor(
                viewTopLeft,
                ImVec2(viewTopLeft.x + cornerLengthX, viewTopLeft.y + thickness),
                opaque, transparent, transparent, opaque);
            drawList->AddRectFilledMultiColor(
                viewTopLeft,
                ImVec2(viewTopLeft.x + thickness, viewTopLeft.y + cornerLengthY),
                opaque, opaque, transparent, transparent);

            // Top-right
            ImVec2 topRight = ImVec2(viewBottomRight.x, viewTopLeft.y);
            drawList->AddRectFilledMultiColor(
                ImVec2(topRight.x - cornerLengthX, topRight.y),
                ImVec2(topRight.x, topRight.y + thickness),
                transparent, opaque, opaque, transparent);
            drawList->AddRectFilledMultiColor(
                topRight,
                ImVec2(topRight.x + thickness, topRight.y + cornerLengthY),
                opaque, opaque, transparent, transparent);

            // Bottom-left
            ImVec2 bottomLeft = ImVec2(viewTopLeft.x, viewBottomRight.y);
            drawList->AddRectFilledMultiColor(
                ImVec2(bottomLeft.x, bottomLeft.y - thickness),
                ImVec2(bottomLeft.x + cornerLengthX, bottomLeft.y),
                opaque, transparent, transparent, opaque);
            drawList->AddRectFilledMultiColor(
                ImVec2(bottomLeft.x, bottomLeft.y - cornerLengthY),
                ImVec2(bottomLeft.x + thickness, bottomLeft.y),
                transparent, transparent, opaque, opaque);

            // Bottom-right
            ImVec2 bottomRight = viewBottomRight;
            drawList->AddRectFilledMultiColor(
                ImVec2(bottomRight.x - cornerLengthX, bottomRight.y - thickness),
                ImVec2(bottomRight.x, bottomRight.y),
                transparent, opaque, opaque, transparent);
            drawList->AddRectFilledMultiColor(
                ImVec2(bottomRight.x, bottomRight.y - cornerLengthY),
                ImVec2(bottomRight.x + thickness, bottomRight.y),
                transparent, transparent, opaque, opaque);
        }
    }
}

void LevelViewer::drawInfo(Level& level, const ImRect& levelRect, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    std::string mouseOnLevelInfo = "None";
    if (levelRect.Contains(ImGui::GetMousePos())) {
        ImVec2 mouseOnLevel = transformPoint(ImGui::GetMousePos(), levelRect, {ImVec2(0, 0), ImVec2(level.data().background->w, level.data().background->h)});
        mouseOnLevelInfo = std::format("{}x{}", (int)mouseOnLevel.x, (int)mouseOnLevel.y);
    }

    auto infoMessage =
        std::format("Size: {}x{}\n"
                    "Pack: {}\n"
                    "Internal location: {}\n"
                    "Exit to global map: {}\n"
                    "\n"
                    "Big cells: {}x{}\n"
                    "Masks: {}\n"
                    "Statics: {}\n"
                    "Animations: {}\n"
                    "Triggers: {}\n"
                    "Floors: {}\n"
                    "\n"
                    "Mouse on level: {}",
                    level.data().background->w, level.data().background->h,
                    level.data().sefData.pack,
                    level.data().sefData.internalLocation,
                    level.data().sefData.exitToGlobalMap,
                    level.data().lvlData.mapTiles.chunkWidth, level.data().lvlData.mapTiles.chunkHeight,
                    level.data().lvlData.maskDescriptions.size(),
                    level.data().lvlData.staticDescriptions.size(),
                    level.data().lvlData.animationDescriptions.size(),
                    level.data().lvlData.triggerDescription.size(),
                    level.data().lvlData.levelFloors,
                    mouseOnLevelInfo);

    const float offset = 4.0f;
    const ImVec2 textSize = ImGui::CalcTextSize(infoMessage.c_str());
    const ImVec2 infoSize = {std::max(textSize.x + offset, 200.0f), textSize.y + offset};

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(
        ImVec2(drawPosition.x, drawPosition.y),
        ImVec2(drawPosition.x + infoSize.x, drawPosition.y + infoSize.y),
        IM_COL32(0, 0, 0, 96));

    ImGui::SetCursorScreenPos({drawPosition.x + offset, drawPosition.y + offset});
    ImGui::Text("%s", infoMessage.c_str());
}

// Чанки и тайлы начинают отсчёт слева сверху и дальше идут по столбцам:
// [0] [2]
// [1] [3]
void LevelViewer::drawMapTiles(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    const MapTiles& mapTiles = level.data().lvlData.mapTiles;
    if (mapTiles.chunks.empty()) return;

    ImGui::SetCursorScreenPos(drawPosition);

    const int chunkSize = 2; // 2x2 tiles per chunk
    const int chunksPerColumn = static_cast<int>(mapTiles.chunkHeight);
    const int chunksPerRow = static_cast<int>(mapTiles.chunkWidth);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 clipMin = ImGui::GetCurrentWindow()->InnerRect.Min;
    ImVec2 clipMax = ImVec2(clipMin.x + ImGui::GetContentRegionAvail().x,
                            clipMin.y + ImGui::GetContentRegionAvail().y);

    const int chunkPixelWidth = chunkSize * Level::tileWidth;
    const int chunkPixelHeight = chunkSize * Level::tileHeight;

    // Вычисляем диапазон видимых чанков
    int minVisibleCol = std::max(0, static_cast<int>((clipMin.x - drawPosition.x) / chunkPixelWidth));
    int maxVisibleCol = std::min(chunksPerRow - 1, static_cast<int>((clipMax.x - drawPosition.x) / chunkPixelWidth));

    int minVisibleRow = std::max(0, static_cast<int>((clipMin.y - drawPosition.y) / chunkPixelHeight));
    int maxVisibleRow = std::min(chunksPerColumn - 1, static_cast<int>((clipMax.y - drawPosition.y) / chunkPixelHeight));

    for (int col = minVisibleCol; col <= maxVisibleCol; ++col)
    {
        for (int row = minVisibleRow; row <= maxVisibleRow; ++row)
        {
            size_t chunkIndex = col * chunksPerColumn + row;
            if (chunkIndex >= mapTiles.chunks.size()) continue;

            const MapChunk& chunk = mapTiles.chunks[chunkIndex];

            int correctX = col * chunkPixelWidth;
            int correctY = row * chunkPixelHeight;

            for (size_t tileIndex = 0; tileIndex < chunk.size(); ++tileIndex) {
                const MapTile& tile = chunk[tileIndex];

                int tileX = correctX + static_cast<int>(tileIndex / chunkSize) * Level::tileWidth;
                int tileY = correctY + static_cast<int>(tileIndex % chunkSize) * Level::tileHeight;

                ImVec2 p0 = ImVec2(drawPosition.x + tileX, drawPosition.y + tileY);
                ImVec2 p1 = ImVec2(p0.x + Level::tileWidth, p0.y + Level::tileHeight);

                ImU32 color;
                MapTilesMode mapTilesMode = level.data().imgui.mapTilesMode;
                if (mapTilesMode == MapTilesMode::Relief) {
                    if (tile.relief <= 1000) {
                        color = IM_COL32(0, 255, 0, 64);
                    } else if (tile.relief <= 2000) {
                        color = IM_COL32(0, 140, 0, 88);
                    } else if (tile.relief <= 15000) {
                        color = IM_COL32(140, 0, 0, 88);
                    } else if (tile.relief <= 35000) {
                        color = IM_COL32(180, 0, 0, 96);
                    } else if (tile.relief <= 55000) {
                        color = IM_COL32(220, 0, 0, 96);
                    } else {
                        color = IM_COL32(255, 0, 0, 104);
                    }
                } else if (mapTilesMode == MapTilesMode::Sound) {
                    if (tile.sound == MapDataSound::Ground) {
                        color = IM_COL32(0, 0, 0, 96);
                    } else if (tile.sound == MapDataSound::Grass) {
                        color = IM_COL32(0, 204, 0, 96);
                    } else if (tile.sound == MapDataSound::Sand) {
                        color = IM_COL32(255, 220, 0, 96);
                    } else if (tile.sound == MapDataSound::Wood) {
                        color = IM_COL32(160, 82, 45, 96);
                    } else if (tile.sound == MapDataSound::Stone) {
                        color = IM_COL32(128, 128, 128, 96);
                    } else if (tile.sound == MapDataSound::Water) {
                        color = IM_COL32(70, 130, 180, 96);
                    } else if (tile.sound == MapDataSound::Snow) {
                        color = IM_COL32(255, 255, 255, 96);
                    }
                } else if (mapTilesMode == MapTilesMode::Mask) {
                    if (tile.mask == 0xffff) {
                        color = IM_COL32(255, 0, 0, 96);
                    } else {
                        color = IM_COL32(0, 0, 0, 96);

                        ImGui::SetCursorScreenPos({p0.x, p0.y});
                        ImGui::PushFont(NULL, 10.0f);
                        ImGui::Text("%u", tile.mask);
                        ImGui::PopFont();
                    }
                }

                drawList->AddRectFilled(p0, p1, color);

                ImVec2 mousePos = ImGui::GetMousePos();
                if (!level.data().imgui.minimapHovered &&
                    ImGui::IsWindowFocused() &&
                    ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                    mousePos.x >= p0.x && mousePos.x < p1.x &&
                    mousePos.y >= p0.y && mousePos.y < p1.y)
                {
                    drawList->AddRect(p0, p1, IM_COL32(255, 255, 0, 255));

                    ImGui::SetTooltip("%dx%d\n"
                                      "Relief: %u\n"
                                      "Sound: %s (%u)\n"
                                      "Mask: %u",
                                      (tileX / Level::tileWidth), (tileY / Level::tileHeight),
                                      tile.relief,
                                      maskSoundToString(static_cast<MapDataSound>(tile.sound)).c_str(), tile.sound,
                                      tile.mask);
                }
            }

            ImVec2 chunkMin = ImVec2(drawPosition.x + correctX, drawPosition.y + correctY);
            ImVec2 chunkMax = ImVec2(chunkMin.x + chunkPixelWidth, chunkMin.y + chunkPixelHeight);

            ImVec2 mousePos = ImGui::GetMousePos();
            if (!level.data().imgui.minimapHovered &&
                ImGui::IsWindowFocused() &&
                ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                mousePos.x >= chunkMin.x && mousePos.x < chunkMax.x &&
                mousePos.y >= chunkMin.y && mousePos.y < chunkMax.y)
            {
                drawList->AddRect(chunkMin, chunkMax, IM_COL32(255, 228, 0, 180));
            }
        }
    }
}

void LevelViewer::drawPersons(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (const SEF_Person& person : level.data().sefData.persons) {
        ImVec2 position(drawPosition.x + person.position.x * Level::tileWidth,
                        drawPosition.y + person.position.y * Level::tileHeight);

        bool fullAlpha = true;
        ImVec2 mousePos = ImGui::GetMousePos();
        if (!level.data().imgui.minimapHovered &&
            ImGui::IsWindowFocused() &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            mousePos.x >= position.x && mousePos.x < (position.x + Level::tileWidth) &&
            mousePos.y >= position.y && mousePos.y < (position.y + Level::tileHeight))
        {
            ImGui::SetTooltip("%s", personInfo(person).c_str());
        } else if (!level.data().imgui.minimapHovered &&
                   ImGui::IsWindowFocused() &&
                   ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            fullAlpha = false;
        }

        drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(255, 228, 0, fullAlpha ? 192 : 64));
        drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));

        const ImVec2 textPos = {position.x + Level::tileWidth + 2.0f, position.y + 4.0f};
        const ImVec2 textSize = ImGui::CalcTextSize(person.literaryName.c_str());
        drawList->AddRectFilled(textPos, {textPos.x + textSize.x, textPos.y + textSize.y}, IM_COL32(0, 0, 0, fullAlpha ? 164 : 48));

        ImGui::SetCursorScreenPos(textPos);
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.0f, fullAlpha ? 1.0f : 0.4f), "%s", person.literaryName.c_str());
    }
}

void LevelViewer::drawPointsEntrance(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (const SEF_PointEntrance& pointEnt : level.data().sefData.pointsEntrance) {
        ImVec2 position(drawPosition.x + pointEnt.position.x * Level::tileWidth,
                        drawPosition.y + pointEnt.position.y * Level::tileHeight);

        bool fullAlpha = true;
        ImVec2 mousePos = ImGui::GetMousePos();
        if (!level.data().imgui.minimapHovered &&
            ImGui::IsWindowFocused() &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            mousePos.x >= position.x && mousePos.x < (position.x + Level::tileWidth) &&
            mousePos.y >= position.y && mousePos.y < (position.y + Level::tileHeight))
        {
            ImGui::SetTooltip("Position: %dx%d\n"
                              "Direction: %s",
                              pointEnt.position.x, pointEnt.position.y,
                              pointEnt.direction.c_str());
        } else if (!level.data().imgui.minimapHovered &&
                   ImGui::IsWindowFocused() &&
                   ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            fullAlpha = false;
        }

        drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(204, 153, 204, fullAlpha ? 192 : 64));
        drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));

        const ImVec2 textPos = {position.x + Level::tileWidth + 2.0f, position.y + 4.0f};
        const ImVec2 textSize = ImGui::CalcTextSize(pointEnt.techName.c_str());
        drawList->AddRectFilled(textPos, {textPos.x + textSize.x, textPos.y + textSize.y}, IM_COL32(0, 0, 0, fullAlpha ? 164 : 48));

        ImGui::SetCursorScreenPos(textPos);
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.8f, fullAlpha ? 1.0f : 0.4f), "%s", pointEnt.techName.c_str());
    }
}

void LevelViewer::drawCellGroups(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto drawListFlags = drawList->Flags;
    drawList->Flags = ImDrawListFlags_None;

    for (int groupIndex = 0; groupIndex < level.data().sefData.cellGroups.size(); ++groupIndex) {
        const SEF_CellGroup& group = level.data().sefData.cellGroups[groupIndex];
        if (group.cells.empty()) break;

        bool fullAlpha = true;
        if (level.data().imgui.highlightCellGroudIndex) {
            fullAlpha = level.data().imgui.highlightCellGroudIndex == groupIndex;
        }

        std::vector<ImVec2> centers;
        for (int i = 0; i < group.cells.size(); ++i) {
            const TilePosition& cellPosition = group.cells[i];

            ImVec2 position(drawPosition.x + cellPosition.x * Level::tileWidth,
                            drawPosition.y + cellPosition.y * Level::tileHeight);

            centers.push_back({position.x + Level::tileWidth / 2, position.y + Level::tileHeight / 2});
        }
        drawList->AddPolyline(&centers[0], centers.size(), IM_COL32(0, 0, 0, fullAlpha ? 180 : 64), 0, 2.0f);

        for (int cellIndex = 0; cellIndex < group.cells.size(); ++cellIndex) {
            const TilePosition& cellPosition = group.cells[cellIndex];

            ImVec2 position(drawPosition.x + cellPosition.x * Level::tileWidth,
                            drawPosition.y + cellPosition.y * Level::tileHeight);

            ImVec2 mousePos = ImGui::GetMousePos();
            if (!level.data().imgui.minimapHovered &&
                ImGui::IsWindowFocused() &&
                ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                mousePos.x >= position.x && mousePos.x < (position.x + Level::tileWidth) &&
                mousePos.y >= position.y && mousePos.y < (position.y + Level::tileHeight))
            {
                ImGui::SetTooltip("Position: %dx%d\n"
                                  "Group: %s\n"
                                  "Name: cell_%02d\n"
                                  "Count: %zu",
                                  cellPosition.x, cellPosition.y,
                                  group.techName.c_str(),
                                  cellIndex,
                                  group.cells.size());

                level.data().imgui.highlightCellGroudIndex = groupIndex;
            }

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                level.data().imgui.highlightCellGroudIndex = std::nullopt;
            }

            drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(51, 255, 204, fullAlpha ? 192 : 64));
            drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));
        }
    }

    drawList->Flags = drawListFlags;
}

void LevelViewer::drawAnimations(Level& level, ImVec2 drawPosition)
{
    uint64_t nowMs = SDL_GetTicks();
    for (Animation& animation : level.data().animations) {
        // TODO: Не рисовать анимацию которую не видно
        animation.update(nowMs);

        ImVec2 animationPosition{drawPosition.x + animation.description.position.x,
                                 drawPosition.y + animation.description.position.y};
        ImGui::SetCursorScreenPos(animationPosition);

        const Texture& texture = animation.currentTexture();
        ImGui::Image((ImTextureID)texture.get(), ImVec2(texture->w, texture->h));

        ImRect animationBox = {animationPosition, {animationPosition.x + texture->w, animationPosition.y + texture->h}};
        if (!level.data().imgui.minimapHovered &&
            ImGui::IsWindowFocused() &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            animationBox.Contains(ImGui::GetMousePos())) {

            ImGui::SetTooltip("Name: %s\n"
                              "Index: %u\n"
                              "Frames: %zu\n"
                              "Duration: %u\n"
                              "Params: %u %u",
                              animation.description.name.c_str(),
                              animation.description.number,
                              animation.textures.size(),
                              animation.duration,
                              animation.description.param1, animation.description.param2);
        }
    }
}

std::string LevelViewer::maskSoundToString(MapDataSound sound)
{
    switch (sound) {
        case MapDataSound::Ground: return "Ground";
        case MapDataSound::Grass:  return "Grass";
        case MapDataSound::Sand:   return "Sand";
        case MapDataSound::Wood:   return "Wood";
        case MapDataSound::Stone:  return "Stone";
        case MapDataSound::Water:  return "Water";
        case MapDataSound::Snow:   return "Snow";
    }
    return {};
}

std::string LevelViewer::personInfo(const SEF_Person& person)
{
    std::string routeString =
        person.route.empty() ? std::string{}
                             : std::format("Route: {}\n", person.route);

    std::string dialogString =
        person.scriptDialog.empty() ? std::string{}
                                    : std::format("Dialog: {}\n", person.scriptDialog);

    std::string inventoryString =
        person.scriptInventory.empty() ? std::string{}
                                       : std::format("Inventory: {}", person.scriptInventory);

    return std::format("Name: {}\n"
                       "Position: {}x{}\n"
                       "Index: {}\n"
                       "Direction: {}\n"
                       "RouteType: {}\n"
                       "{}"
                       "Radius: {}\n"
                       "DelayMin: {}\n"
                       "DelayMax: {}\n"
                       "Tribe: {}\n"
                       "{}"
                       "{}",
                       person.techName,
                       person.position.x, person.position.y,
                       person.literaryNameIndex,
                       person.direction,
                       person.routeType,
                       routeString,
                       person.radius,
                       person.delayMin,
                       person.delayMax,
                       person.tribe,
                       dialogString,
                       inventoryString);
}
