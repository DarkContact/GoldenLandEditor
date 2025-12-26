#include "LevelViewer.h"

#include <format>

#include <SDL3/SDL_timer.h>
#include "imgui_internal.h"
#include "imgui.h"

#include "utils/ImGuiWidgets.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"
#include "utils/TracyProfiler.h"


LevelViewer::LevelViewer() {}

void LevelViewer::update(bool& showWindow, std::string_view rootDirectory, Level& level)
{
    Tracy_ZoneScoped;
    auto levelWindowName = Level::levelWindowName(level.data().name, level.data().type);
    Tracy_ZoneText(levelWindowName.c_str(), levelWindowName.size());

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool isWindowVisible = ImGui::Begin(levelWindowName.c_str(), &showWindow, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    if (!isWindowVisible) {
        level.data().imgui.hasVisibleAnimations = false;
        ImGui::End();
        return;
    }

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
            if (ImGui::MenuItem("Sounds", "S", level.data().imgui.showSounds)) {
                level.data().imgui.showSounds = !level.data().imgui.showSounds;
            }
            if (ImGui::MenuItem("Triggers", "Alt", level.data().imgui.showTriggers)) {
                level.data().imgui.showTriggers = !level.data().imgui.showTriggers;
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

        if (level.data().imgui.showCellGroups && ImGui::BeginMenu("Cell group mode")) {
            if (ImGui::MenuItem("Both", "Shift + 1", level.data().imgui.cellGroupMode == CellGroupMode::Both)) {
                level.data().imgui.cellGroupMode = CellGroupMode::Both;
            }
            if (ImGui::MenuItem("Only SEF", "Shift + 2", level.data().imgui.cellGroupMode == CellGroupMode::OnlySef)) {
                level.data().imgui.cellGroupMode = CellGroupMode::OnlySef;
            }
            if (ImGui::MenuItem("Only LVL", "Shift + 3", level.data().imgui.cellGroupMode == CellGroupMode::OnlyLvl)) {
                level.data().imgui.cellGroupMode = CellGroupMode::OnlyLvl;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Files")) {
            if (ImGui::MenuItem("Open level folder", NULL)) {
                std::string error;
                std::string levelMainDir = Level::levelMainDir(rootDirectory, levelTypeToString(level.data().type), level.data().name);
                if (!FileUtils::openFolderAndSelectItems(levelMainDir, {}, &error)) {
                    Log(error);
                }
            }
            if (ImGui::MenuItem("Open pack folder", NULL)) {
                std::string error;
                std::string levelPackDir = Level::levelPackDir(rootDirectory, level.data().sefData.pack);
                if (!FileUtils::openFolderAndSelectItems(levelPackDir, {}, &error)) {
                    Log(error);
                }
            }
            if (ImGui::MenuItem("Open .lvl", NULL)) {
                std::string error;
                std::string levelLvlDir = Level::levelLvlDir(rootDirectory);
                std::string levelLvlFile = Level::levelLvl(rootDirectory, level.data().sefData.pack);
                std::array<std::string_view, 1> files = {levelLvlFile};
                if (!FileUtils::openFolderAndSelectItems(levelLvlDir, files, &error)) {
                    Log(error);
                }
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
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_S, false)) {
            level.data().imgui.showSounds = !level.data().imgui.showSounds;
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiMod_Alt, false)) {
            level.data().imgui.showTriggers = !level.data().imgui.showTriggers;
        }

        ImGuiIO& io = ImGui::GetIO();
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

        if (level.data().imgui.showCellGroups) {
            if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_1, false)) {
                level.data().imgui.cellGroupMode = CellGroupMode::Both;
            }
            if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_2, false)) {
                level.data().imgui.cellGroupMode = CellGroupMode::OnlySef;
            }
            if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_3, false)) {
                level.data().imgui.cellGroupMode = CellGroupMode::OnlyLvl;
            }
        }

        // TODO: Добавить плавности
        const float scrollStep = 8.0f;
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_LeftArrow)) {
            ImGui::SetScrollX(ImGui::GetScrollX() - scrollStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_RightArrow)) {
            ImGui::SetScrollX(ImGui::GetScrollX() + scrollStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_UpArrow)) {
            ImGui::SetScrollY(ImGui::GetScrollY() - scrollStep);
        }
        if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_DownArrow)) {
            ImGui::SetScrollY(ImGui::GetScrollY() + scrollStep);
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
        ImGui::SetWindowFocus();
    }

    handleLevelDragScroll(level);

    // Отрисовка уровня
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)level.data().background.get(), ImVec2(level.data().background->w, level.data().background->h));

    if (level.data().imgui.showAnimations) {
        drawAnimations(level, startPos);
    }
    if (level.data().imgui.showPersons) {
        drawPersons(level, startPos);
    }
    if (level.data().imgui.showEntrancePoints) {
        drawPointsEntrance(level, startPos);
    }
    if (level.data().imgui.showCellGroups) {
        drawCellGroups(level, startPos);
    }
    if (level.data().imgui.showSounds) {
        drawSounds(level, startPos);
    }
    if (level.data().imgui.showTriggers) {
        drawTriggers(level, startPos);
    }
    if (level.data().imgui.showMapTiles) {
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

    if (level.data().imgui.showMetaInfo) {
        drawInfo(level, levelRect, {minimapRect.GetBL().x, minimapRect.GetBL().y});
    }

    ImGui::End();
}

bool LevelViewer::isAnimating(const Level& level) const
{
    bool showLevelAnimation = level.data().imgui.showAnimations && level.data().imgui.hasVisibleAnimations;
    bool showMinimapAnimation = level.data().imgui.minimapAnimating;
    return showLevelAnimation || showMinimapAnimation;
}

bool LevelViewer::isVisibleInWindow(const ImRect& rect) const
{
    return ImGui::GetCurrentWindow()->ClipRect.Overlaps(rect);
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

bool LevelViewer::leftMouseDownOnLevel(const Level& level) {
    return !level.data().imgui.minimapHovered &&
           ImGui::IsWindowFocused() &&
           ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

const char* LevelViewer::maskSoundToString(MapDataSound sound) {
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

std::string LevelViewer::personInfo(const SEF_Person& person) {
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
                       person.route.empty() ? std::string{}
                                            : std::format("Route: {}\n", person.route),
                       person.radius,
                       person.delayMin,
                       person.delayMax,
                       person.tribe,
                       person.scriptDialog.empty() ? std::string{}
                                                   : std::format("Dialog: {}\n", person.scriptDialog),
                       person.scriptInventory.empty() ? std::string{}
                                                      : std::format("Inventory: {}\n", person.scriptInventory));
}


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
        if (ImGui::IsWindowFocused() && !imgui.draggingMinimap && hoveredView && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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
        else if (ImGui::IsWindowFocused() && !imgui.draggingMinimap && hoveredMinimap && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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
        else if (imgui.minimapAnimating) {
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
                    "Weather: {}\n"
                    "Internal location: {}\n"
                    "Exit to global map: {}\n"
                    "\n"
                    "Tiles: {}x{}\n"
                    "Chunks: {}x{}\n"
                    "\n"
                    "Masks: {}\n"
                    "Statics: {}\n"
                    "Animations: {}\n"
                    "Triggers: {}\n"
                    "Sounds: {}\n"
                    "Floors: {}\n"
                    "\n"
                    "Mouse on level: {}",
                    level.data().background->w, level.data().background->h,
                    level.data().sefData.pack,
                    level.data().sefData.weather.value_or(-1),
                    level.data().sefData.internalLocation,
                    level.data().sefData.exitToGlobalMap,
                    level.data().lvlData.mapTiles.chunkWidth * 2, level.data().lvlData.mapTiles.chunkHeight * 2,
                    level.data().lvlData.mapTiles.chunkWidth, level.data().lvlData.mapTiles.chunkHeight,
                    level.data().lvlData.maskDescriptions.size(),
                    level.data().lvlData.staticDescriptions.size(),
                    level.data().lvlData.animationDescriptions.size(), // TODO: LaoSize, FileCount
                    level.data().lvlData.triggerDescriptions.size(),
                    level.data().lvlData.sounds.otherSounds.size(),
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
    ImGui::TextColored(ImVec4{1.0f, 1.0f, 1.0f, 1.0f}, "%s", infoMessage.c_str());
}

// Порядок отрисовки чанков и тайлов:
// [0] [2]
// [1] [3]
void LevelViewer::drawMapTiles(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const MapTiles& mapTiles = level.data().lvlData.mapTiles;
    if (mapTiles.chunks.empty()) return;

    ImGui::SetCursorScreenPos(drawPosition);

    const int chunkSize = 2; // 2x2 tiles per chunk
    const int chunksPerColumn = mapTiles.chunkHeight;
    const int chunksPerRow = mapTiles.chunkWidth;

    ImVec2 clipMin = ImGui::GetCurrentWindow()->InnerRect.Min;
    ImVec2 clipMax = ImVec2(clipMin.x + ImGui::GetContentRegionAvail().x,
                            clipMin.y + ImGui::GetContentRegionAvail().y);

    // Вычисляем диапазон видимых чанков
    int minVisibleColumn = std::max(0, static_cast<int>((clipMin.x - drawPosition.x) / Level::chunkWidth));
    int maxVisibleColumn = std::min(chunksPerRow - 1, static_cast<int>((clipMax.x - drawPosition.x) / Level::chunkWidth));

    int minVisibleRow = std::max(0, static_cast<int>((clipMin.y - drawPosition.y) / Level::chunkHeight));
    int maxVisibleRow = std::min(chunksPerColumn - 1, static_cast<int>((clipMax.y - drawPosition.y) / Level::chunkHeight));

    for (int chunkColumn = minVisibleColumn; chunkColumn <= maxVisibleColumn; ++chunkColumn) {
        for (int chunkRow = minVisibleRow; chunkRow <= maxVisibleRow; ++chunkRow) {
            size_t chunkIndex = chunkColumn * chunksPerColumn + chunkRow;
            const MapChunk& chunk = mapTiles.chunks[chunkIndex];

            bool allSameColor = true;
            ImU32 firstColor = getTileColor(chunk.front(), level.data().imgui.mapTilesMode);
            for (size_t i = 1; i < chunk.size(); ++i) {
                ImU32 color = getTileColor(chunk[i], level.data().imgui.mapTilesMode);
                if (firstColor != color) {
                    allSameColor = false;
                    break;
                }
            }

            ImVec2 chunkTopLeft = ImVec2(drawPosition.x + (chunkColumn * Level::chunkWidth),
                                         drawPosition.y + (chunkRow * Level::chunkHeight));

            if (allSameColor) {
                ImVec2 chunkBottomRight = ImVec2(chunkTopLeft.x + Level::chunkWidth,
                                                 chunkTopLeft.y + Level::chunkHeight);
                drawList->AddRectFilled(chunkTopLeft, chunkBottomRight, firstColor);
            }

            for (size_t tileIndex = 0; tileIndex < chunk.size(); ++tileIndex) {
                const MapTile& tile = chunk[tileIndex];

                int tileColumn = chunkColumn * chunkSize + (tileIndex / chunkSize);
                int tileRow = chunkRow * chunkSize + (tileIndex % chunkSize);

                ImVec2 tileTopLeft = ImVec2(drawPosition.x + (tileColumn * Level::tileWidth),
                                            drawPosition.y + (tileRow * Level::tileHeight));

                ImVec2 tileBottomRight = ImVec2(tileTopLeft.x + Level::tileWidth,
                                                tileTopLeft.y + Level::tileHeight);

                if (!allSameColor) {
                    ImU32 color = getTileColor(tile, level.data().imgui.mapTilesMode);
                    drawList->AddRectFilled(tileTopLeft, tileBottomRight, color);
                }

                if (level.data().imgui.mapTilesMode == MapTilesMode::Mask && tile.mask != MapTile::kEmptyMask) {
                    ImGui::SetCursorScreenPos({tileTopLeft.x, tileTopLeft.y});

                    ImGui::PushFont(NULL, 10.0f);
                    ImGui::Text("%u", tile.mask);
                    ImGui::PopFont();
                }

                drawTileBorderAndTooltip(tile, tileTopLeft, tileBottomRight, tileColumn, tileRow, chunkColumn, chunkRow, level);
            }

            drawChunkBorder(chunkTopLeft, level);
        }
    }
}

ImU32 LevelViewer::getTileColor(const MapTile& tile, MapTilesMode mode) {
    switch (mode) {
        case MapTilesMode::Relief:
            if (tile.relief <= 1000)        return IM_COL32(0, 255, 0, 64);
            else if (tile.relief <= 2000)   return IM_COL32(0, 140, 0, 88);
            else if (tile.relief <= 15000)  return IM_COL32(140, 0, 0, 88);
            else if (tile.relief <= 35000)  return IM_COL32(180, 0, 0, 96);
            else if (tile.relief <= 55000)  return IM_COL32(220, 0, 0, 96);
            else                            return IM_COL32(255, 0, 0, 104);
        case MapTilesMode::Sound:
            switch (tile.sound) {
                case MapDataSound::Ground: return IM_COL32(0, 0, 0, 96);
                case MapDataSound::Grass:  return IM_COL32(0, 204, 0, 96);
                case MapDataSound::Sand:   return IM_COL32(255, 220, 0, 96);
                case MapDataSound::Wood:   return IM_COL32(160, 82, 45, 96);
                case MapDataSound::Stone:  return IM_COL32(128, 128, 128, 96);
                case MapDataSound::Water:  return IM_COL32(70, 130, 180, 96);
                case MapDataSound::Snow:   return IM_COL32(255, 255, 255, 96);
            }
        case MapTilesMode::Mask:
            return tile.mask == MapTile::kEmptyMask ? IM_COL32(255, 0, 0, 96)
                                                    : IM_COL32(0, 0, 0, 96);
    }
    return IM_COL32(255, 255, 255, 255);
}

void LevelViewer::drawTileBorderAndTooltip(const MapTile& tile, ImVec2 tileTopLeft, ImVec2 tileBottomRight, int tileColumn, int tileRow, int chunkColumn, int chunkRow, Level& level)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 mousePos = ImGui::GetMousePos();
    if (leftMouseDownOnLevel(level) &&
        mousePos.x >= tileTopLeft.x && mousePos.x < tileBottomRight.x &&
        mousePos.y >= tileTopLeft.y && mousePos.y < tileBottomRight.y)
    {
        drawList->AddRect(tileTopLeft, tileBottomRight, IM_COL32(255, 255, 0, 255));

        ImGui::SetTooltip("[MAP TILE]\n"
                          "Tile: %dx%d\n"
                          "Chunk: %dx%d\n"
                          "\n"
                          "Relief: %u\n"
                          "Sound: %s (%u)\n"
                          "Mask: %u",
                          tileColumn, tileRow,
                          chunkColumn, chunkRow,
                          tile.relief,
                          maskSoundToString(static_cast<MapDataSound>(tile.sound)), tile.sound,
                          tile.mask);
    }
}

void LevelViewer::drawChunkBorder(ImVec2 chunkTopLeft, Level& level)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 chunkBottomRight = ImVec2(chunkTopLeft.x + Level::chunkWidth,
                                     chunkTopLeft.y + Level::chunkHeight);

    ImVec2 mousePos = ImGui::GetMousePos();
    if (leftMouseDownOnLevel(level) &&
        mousePos.x >= chunkTopLeft.x && mousePos.x < chunkBottomRight.x &&
        mousePos.y >= chunkTopLeft.y && mousePos.y < chunkBottomRight.y)
    {
        drawList->AddRect(chunkTopLeft, chunkBottomRight, IM_COL32(255, 228, 0, 180));
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
        ImRect personBox = {position, {position.x + Level::tileWidth, position.y + Level::tileHeight}};
        if (leftMouseDownOnLevel(level) &&
            personBox.Contains(ImGui::GetMousePos()))
        {
            ImGuiWidgets::SetTooltipStacked("[PERSON]\n%s", personInfo(person).c_str());
        } else if (leftMouseDownOnLevel(level)) {
            fullAlpha = false;
        }

        if (isVisibleInWindow(personBox)) {
            drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(255, 228, 0, fullAlpha ? 192 : 64));
            drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));
        }

        std::string_view personName = level.data().sdbData.strings.empty() ? person.techName
                                                                           : level.data().sdbData.strings.at(person.literaryNameIndex);

        const ImVec2 textPos = {position.x + Level::tileWidth + 2.0f, position.y + 4.0f};
        const ImVec2 textSize = ImGui::CalcTextSize(personName.data());
        ImRect textBox = {textPos, {textPos.x + textSize.x, textPos.y + textSize.y}};

        if (isVisibleInWindow(textBox)) {
            drawList->AddRectFilled(textBox.Min, textBox.Max, IM_COL32(0, 0, 0, fullAlpha ? 164 : 48));
            ImGui::SetCursorScreenPos(textPos);
            ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.0f, fullAlpha ? 1.0f : 0.4f), "%s", personName.data());
        }
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
        ImRect pointBox = {position, {position.x + Level::tileWidth, position.y + Level::tileHeight}};
        if (leftMouseDownOnLevel(level) &&
            pointBox.Contains(ImGui::GetMousePos()))
        {
            ImGuiWidgets::SetTooltipStacked("[POINT ENTRANCE]\n"
                                            "Name: %s\n"
                                            "Position: %dx%d\n"
                                            "Direction: %s",
                                            pointEnt.techName.c_str(),
                                            pointEnt.position.x, pointEnt.position.y,
                                            pointEnt.direction.c_str());
        } else if (leftMouseDownOnLevel(level)) {
            fullAlpha = false;
        }

        if (isVisibleInWindow(pointBox)) {
            drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(204, 153, 204, fullAlpha ? 192 : 64));
            drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));
        }

        const ImVec2 textPos = {position.x + Level::tileWidth + 2.0f, position.y + 4.0f};
        const ImVec2 textSize = ImGui::CalcTextSize(pointEnt.techName.c_str());
        ImRect textBox = {textPos, {textPos.x + textSize.x, textPos.y + textSize.y}};

        if (isVisibleInWindow(textBox)) {
            drawList->AddRectFilled(textPos, {textPos.x + textSize.x, textPos.y + textSize.y}, IM_COL32(0, 0, 0, fullAlpha ? 164 : 48));
            ImGui::SetCursorScreenPos(textPos);
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.8f, fullAlpha ? 1.0f : 0.4f), "%s", pointEnt.techName.c_str());
        }
    }
}

void LevelViewer::drawCellGroups(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto drawListFlags = drawList->Flags;
    drawList->Flags &= ~ImDrawListFlags_AntiAliasedLines; // Disable flag

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        level.data().imgui.highlightCellGroudIndex = std::nullopt;
    }

    bool drawSef = level.data().imgui.cellGroupMode == CellGroupMode::Both
                   || level.data().imgui.cellGroupMode == CellGroupMode::OnlySef;

    bool drawLvl = level.data().imgui.cellGroupMode == CellGroupMode::Both
                   || level.data().imgui.cellGroupMode == CellGroupMode::OnlyLvl;

    int groupIndex = 0;
    if (drawSef) {
        for (const CellGroup& group : level.data().sefData.cellGroups) {
            if (group.cells.empty()) continue;

            drawCellGroup(level.data().imgui, drawPosition, group, groupIndex, SDL_Color{51, 255, 204, 192}, true);
            ++groupIndex;
        }
    }

    groupIndex = level.data().sefData.cellGroups.size();
    if (drawLvl) {
        for (const CellGroup& group : level.data().lvlData.cellGroups) {
            if (group.cells.empty()) continue;

            drawCellGroup(level.data().imgui, drawPosition, group, groupIndex, SDL_Color{40, 200, 200, 192}, false);
            ++groupIndex;
        }
    }

    drawList->Flags = drawListFlags;
}

void LevelViewer::drawCellGroup(LevelImgui& imgui, ImVec2 drawPosition, const CellGroup& group, int groupIndex, SDL_Color color, bool drawConnectedLine)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    bool fullAlpha = true;
    if (imgui.highlightCellGroudIndex) {
        fullAlpha = imgui.highlightCellGroudIndex == groupIndex;
    }

    if (drawConnectedLine && group.cells.size() >= 2) {
        std::vector<ImVec2> centers;
        centers.reserve(group.cells.size());
        for (const TilePosition& cellPosition : group.cells) {
            ImVec2 position(drawPosition.x + cellPosition.x * Level::tileWidth,
                            drawPosition.y + cellPosition.y * Level::tileHeight);

            centers.push_back({position.x + Level::tileWidth / 2.0f, position.y + Level::tileHeight / 2.0f});
        }

        ImRect polylineRect(centers[0], centers[0]);
        for (const ImVec2& p : centers) {
            polylineRect.Min = ImMin(polylineRect.Min, p);
            polylineRect.Max = ImMax(polylineRect.Max, p);
        }

        const float thickness = 2.0f;
        polylineRect.Expand(thickness);

        if (isVisibleInWindow(polylineRect)) {
            drawList->AddPolyline(centers.data(), centers.size(), IM_COL32(0, 0, 0, fullAlpha ? (color.a - 36) : 64), 0, thickness);
        }
    }

    for (int cellIndex = 0; cellIndex < group.cells.size(); ++cellIndex) {
        const TilePosition& cellPosition = group.cells[cellIndex];

        ImVec2 position(drawPosition.x + cellPosition.x * Level::tileWidth,
                        drawPosition.y + cellPosition.y * Level::tileHeight);

        ImRect cellBox = {position, {position.x + Level::tileWidth, position.y + Level::tileHeight}};
        if (!isVisibleInWindow(cellBox)) continue;

        if (!imgui.minimapHovered &&
            ImGui::IsWindowFocused() &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            cellBox.Contains(ImGui::GetMousePos()))
        {
            ImGuiWidgets::SetTooltipStacked("[CELL GROUP]\n"
                                            "Name: %s\n"
                                            "Position: %dx%d\n"
                                            "Index: %d\n"
                                            "Count: %zu",
                                            group.name.c_str(),
                                            cellPosition.x, cellPosition.y,
                                            cellIndex,
                                            group.cells.size());

            imgui.highlightCellGroudIndex = groupIndex;
        }

        drawList->AddRectFilled(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(color.r, color.g, color.b, fullAlpha ? color.a : 64));
        drawList->AddRect(position, {position.x + Level::tileWidth, position.y + Level::tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? color.a : 64));
    }
}

void LevelViewer::drawAnimations(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    bool hasVisibleAnimations = false;

    uint64_t nowMs = SDL_GetTicks();
    for (LevelAnimation& animation : level.data().animations) {
        animation.update(nowMs);

        if (animation.textures.empty()) { continue; }

        ImVec2 animationPosition{drawPosition.x + animation.description.position.x,
                                 drawPosition.y + animation.description.position.y};
        const Texture& texture = animation.currentTexture();
        ImRect animationBox = {animationPosition, {animationPosition.x + texture->w, animationPosition.y + texture->h}};

        if (!isVisibleInWindow(animationBox)) { continue; }

        hasVisibleAnimations = true;
        ImGui::SetCursorScreenPos(animationPosition);

        ImGui::Image((ImTextureID)texture.get(), ImVec2(texture->w, texture->h));

        if (leftMouseDownOnLevel(level) &&
            animationBox.Contains(ImGui::GetMousePos())) {

            drawList->AddRect(animationBox.Min, animationBox.Max, IM_COL32(255, 228, 0, 192));

            ImGuiWidgets::SetTooltipStacked("[ANIMATION]\n"
                                            "Name: %s\n"
                                            "Position: %dx%d\n"
                                            "Index: %u\n"
                                            "Size: %dx%d\n"
                                            "Frames: %zu\n"
                                            "Delay: %u\n"
                                            "Params: %u %u",
                                            animation.description.name.c_str(),
                                            animation.description.position.x, animation.description.position.y,
                                            animation.description.number,
                                            animation.textures.front()->w, animation.textures.front()->h,
                                            animation.textures.size(),
                                            animation.delayMs,
                                            animation.description.param1, animation.description.param2);
        }
    }
    level.data().imgui.hasVisibleAnimations = hasVisibleAnimations;
}

void LevelViewer::drawSounds(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (const ExtraSound& sound : level.data().lvlData.sounds.otherSounds) {
        ImVec2 position(drawPosition.x + sound.chunkPositionX * Level::chunkWidth,
                        drawPosition.y + sound.chunkPositionY * Level::chunkHeight);
        ImRect soundBox = {position, {position.x + Level::chunkWidth, position.y + Level::chunkHeight}};

        if (!isVisibleInWindow(soundBox)) continue;

        bool fullAlpha = true;
        if (leftMouseDownOnLevel(level) &&
            soundBox.Contains(ImGui::GetMousePos()))
        {
            ImGuiWidgets::SetTooltipStacked("[SOUND]\n"
                                            "Path: %s\n"
                                            //"Chunk position: %.0fx%.0f\n"
                                            "Param03: %.2f\n"
                                            "Param04: %.2f\n"
                                            "Param05: %.2f\n"
                                            "Param06: %.2f\n"
                                            "Param07: %.2f\n"
                                            "Param08: %.2f\n"
                                            "Param09: %u\n"
                                            "Param10: %u\n"
                                            "Param11: %u\n"
                                            "Param12: %u\n",
                                            sound.path.c_str(),
                                            //sound.chunkPositionX, sound.chunkPositionY,
                                            sound.param03,
                                            sound.param04,
                                            sound.param05,
                                            sound.param06,
                                            sound.param07,
                                            sound.param08,
                                            sound.param09,
                                            sound.param10,
                                            sound.param11,
                                            sound.param12);
        } else if (leftMouseDownOnLevel(level)) {
            fullAlpha = false;
        }

        drawList->AddRectFilled(position, {position.x + Level::chunkWidth, position.y + Level::chunkHeight}, IM_COL32(255, 255, 255, fullAlpha ? 192 : 64));
        drawList->AddRect(position, {position.x + Level::chunkWidth, position.y + Level::chunkHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));
    }
}

void LevelViewer::drawTriggers(Level& level, ImVec2 drawPosition)
{
    Tracy_ZoneScoped;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (const LevelTrigger& trigger : level.data().triggers) {
        ImVec2 triggerPosition{drawPosition.x + trigger.lvlDescription.position.x,
                               drawPosition.y + trigger.lvlDescription.position.y};
        ImRect triggerBox = {triggerPosition, {triggerPosition.x + trigger.texture->w, triggerPosition.y + trigger.texture->h}};

        if (!isVisibleInWindow(triggerBox)) { continue; }

        ImGui::SetCursorScreenPos(triggerPosition);

        int alpha = 64;
        int blendMode = SDL_BLENDMODE_BLEND;
        bool isTransition = false;
        bool isVisible = false;
        bool isActive = false;
        if (trigger.sefDescription) {
            isTransition = trigger.sefDescription->get().isTransition.value_or(false);
            isVisible = trigger.sefDescription->get().isVisible.value_or(true);
            isActive = trigger.sefDescription->get().isActive;
            if (isTransition) {
                alpha = 255;
                blendMode = SDL_BLENDMODE_ADD;
            }
        }

        SDL_SetTextureBlendMode(trigger.texture.get(), blendMode);
        ImVec4 tintColor{1, 1, 1, alpha / 255.0f};
        ImGui::ImageWithBg((ImTextureID)trigger.texture.get(),
                           ImVec2(trigger.texture->w, trigger.texture->h),
                           ImVec2(0, 0), ImVec2(1, 1), {0, 0, 0, 0}, tintColor);

        if (leftMouseDownOnLevel(level) &&
            triggerBox.Contains(ImGui::GetMousePos())) {

            drawList->AddRect(triggerBox.Min, triggerBox.Max, IM_COL32(255, 228, 0, 192));

            std::string_view triggerName = "[NONE]";
            if (!level.data().sdbData.strings.empty() && trigger.sefDescription) {
                triggerName = level.data().sdbData.strings.at(trigger.sefDescription->get().literaryNameIndex);
            }

            ImGuiWidgets::SetTooltipStacked("[TRIGGER]\n"
                                            "Name: %s\n"
                                            "LitName: %s\n"
                                            "Position: %dx%d\n"
                                            "Index: %u\n"
                                            "Size: %dx%d\n"
                                            "Params: %u %u\n"
                                            "IsTransition: %d\n"
                                            "IsVisible: %d\n"
                                            "IsActive: %d",
                                            trigger.lvlDescription.name.c_str(),
                                            triggerName.data(),
                                            trigger.lvlDescription.position.x, trigger.lvlDescription.position.y,
                                            trigger.lvlDescription.number,
                                            trigger.texture->w, trigger.texture->h,
                                            trigger.lvlDescription.param1, trigger.lvlDescription.param2,
                                            isTransition,
                                            isVisible,
                                            isActive);
        }
    }
}
