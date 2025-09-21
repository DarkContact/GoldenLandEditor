#include "LevelViewer.h"

#include <format>

#include "imgui.h"
#include "imgui_internal.h"

bool LevelViewer::update(bool& showWindow, Level& level)
{
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
            if (ImGui::MenuItem("Mask", "M", level.data().imgui.showMask)) {
                level.data().imgui.showMask = !level.data().imgui.showMask;
            }
            if (ImGui::MenuItem("Persons", "P", level.data().imgui.showPersons)) {
                level.data().imgui.showPersons = !level.data().imgui.showPersons;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Tab, false)) {
        level.data().imgui.showMinimap = !level.data().imgui.showMinimap;
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_I, false)) {
        level.data().imgui.showMetaInfo = !level.data().imgui.showMetaInfo;
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_M, false)) {
        level.data().imgui.showMask = !level.data().imgui.showMask;
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_P, false)) {
        level.data().imgui.showPersons = !level.data().imgui.showPersons;
    }

    // Отрисовка уровня
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)level.data().background, ImVec2(level.data().background->w, level.data().background->h));

    // Отрисовка персонажей
    if (level.data().imgui.showPersons)
    {
        drawPersons(level, startPos);
    }

    if (level.data().imgui.showMask)
    {
        drawMask(level, startPos);
    }

    ImRect minimapRect;
    if (level.data().imgui.showMinimap) {
        updateMinimap(level, minimapRect);
        minimapRect.Max.y += 16.0f;
    } else {
        ImVec2 minimapSize = {200.0f, 0.0f};
        ImVec2 minimapPosition = computeMinimapPosition(level, minimapSize);

        minimapRect = {minimapPosition, {minimapPosition.x + minimapSize.x,
                                         minimapPosition.y + minimapSize.y + 8.0f}};
    }

    if (level.data().imgui.showMetaInfo)
    {
        updateInfo(level, {minimapRect.GetBL().x, minimapRect.GetBL().y});
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

void LevelViewer::updateMinimap(Level& level, ImRect& minimapRect)
{
    bool hasMinimap = (level.data().minimap != nullptr);

    const ImVec2 minimapSize = computeMinimapSize(level, hasMinimap);
    const ImVec2 minimapPosition = computeMinimapPosition(level, minimapSize);

    // Миникарта и белая обводка
    {
        ImGui::SetCursorScreenPos(minimapPosition);

        ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

        ImGui::ImageWithBg((ImTextureID)(hasMinimap ? level.data().minimap
                                                    : level.data().background),
                           minimapSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 1));

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
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
        minimapRect = ImRect(minimapPosition, ImVec2(minimapPosition.x + minimapSize.x, minimapPosition.y + minimapSize.y));

        auto& imgui = level.data().imgui;

        bool hoveredView = viewRect.Contains(io.MousePos);
        bool hoveredMinimap = minimapRect.Contains(io.MousePos);

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

            ImDrawList* draw = ImGui::GetWindowDrawList();

            // Top-left
            draw->AddRectFilledMultiColor(
                viewTopLeft,
                ImVec2(viewTopLeft.x + cornerLengthX, viewTopLeft.y + thickness),
                opaque, transparent, transparent, opaque);
            draw->AddRectFilledMultiColor(
                viewTopLeft,
                ImVec2(viewTopLeft.x + thickness, viewTopLeft.y + cornerLengthY),
                opaque, opaque, transparent, transparent);

            // Top-right
            ImVec2 topRight = ImVec2(viewBottomRight.x, viewTopLeft.y);
            draw->AddRectFilledMultiColor(
                ImVec2(topRight.x - cornerLengthX, topRight.y),
                ImVec2(topRight.x, topRight.y + thickness),
                transparent, opaque, opaque, transparent);
            draw->AddRectFilledMultiColor(
                topRight,
                ImVec2(topRight.x + thickness, topRight.y + cornerLengthY),
                opaque, opaque, transparent, transparent);

            // Bottom-left
            ImVec2 bottomLeft = ImVec2(viewTopLeft.x, viewBottomRight.y);
            draw->AddRectFilledMultiColor(
                ImVec2(bottomLeft.x, bottomLeft.y - thickness),
                ImVec2(bottomLeft.x + cornerLengthX, bottomLeft.y),
                opaque, transparent, transparent, opaque);
            draw->AddRectFilledMultiColor(
                ImVec2(bottomLeft.x, bottomLeft.y - cornerLengthY),
                ImVec2(bottomLeft.x + thickness, bottomLeft.y),
                transparent, transparent, opaque, opaque);

            // Bottom-right
            ImVec2 bottomRight = viewBottomRight;
            draw->AddRectFilledMultiColor(
                ImVec2(bottomRight.x - cornerLengthX, bottomRight.y - thickness),
                ImVec2(bottomRight.x, bottomRight.y),
                transparent, opaque, opaque, transparent);
            draw->AddRectFilledMultiColor(
                ImVec2(bottomRight.x, bottomRight.y - cornerLengthY),
                ImVec2(bottomRight.x + thickness, bottomRight.y),
                transparent, transparent, opaque, opaque);
        }
    }
}

void LevelViewer::updateInfo(Level& level, ImVec2 drawPosition)
{
    auto infoMessage =
        std::format("Size: {}x{}\n"
                    "Pack: {}\n"
                    "Big cells: {}x{}\n"
                    "Animations count: {}\n"
                    "Triggers count: {}\n"
                    "Floors: {}",
                    level.data().background->w, level.data().background->h,
                    level.data().sefData.pack,
                    level.data().lvlData.mapData.width, level.data().lvlData.mapData.height,
                    level.data().lvlData.animationDescriptions.size(),
                    level.data().lvlData.triggerDescription.size(),
                    level.data().lvlData.levelFloors);

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
void LevelViewer::drawMask(Level& level, ImVec2 drawPosition)
{
    const MaskHDR& maskHDR = level.data().lvlData.mapData;
    if (maskHDR.chunks.empty()) return;

    ImGui::SetCursorScreenPos(drawPosition);

    const int tileWidth = 12;
    const int tileHeight = 9;
    const int chunkSize = 2; // 2x2 tiles per chunk

    const int chunksPerColumn = static_cast<int>(maskHDR.height);
    const int chunksPerRow = static_cast<int>(maskHDR.width);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 clipMin = ImGui::GetCurrentWindow()->InnerRect.Min;
    ImVec2 clipMax = ImVec2(clipMin.x + ImGui::GetContentRegionAvail().x,
                            clipMin.y + ImGui::GetContentRegionAvail().y);

    const int chunkPixelWidth = chunkSize * tileWidth;
    const int chunkPixelHeight = chunkSize * tileHeight;

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
            if (chunkIndex >= maskHDR.chunks.size()) continue;

            const MHDRChunk& chunk = maskHDR.chunks[chunkIndex];

            int correctX = col * chunkPixelWidth;
            int correctY = row * chunkPixelHeight;

            for (size_t tileIndex = 0; tileIndex < chunk.size(); ++tileIndex) {
                const MHDRTile& tile = chunk[tileIndex];

                int tileX = correctX + static_cast<int>(tileIndex / chunkSize) * tileWidth;
                int tileY = correctY + static_cast<int>(tileIndex % chunkSize) * tileHeight;

                ImVec2 p0 = ImVec2(drawPosition.x + tileX, drawPosition.y + tileY);
                ImVec2 p1 = ImVec2(p0.x + tileWidth, p0.y + tileHeight);

                ImU32 color;
                if (tile.maskNumber <= 1000) {
                    color = IM_COL32(0, 255, 0, 64);
                } else if (tile.maskNumber <= 2000) {
                    color = IM_COL32(0, 140, 0, 88);
                } else if (tile.maskNumber <= 15000) {
                    color = IM_COL32(140, 0, 0, 88);
                } else if (tile.maskNumber <= 35000) {
                    color = IM_COL32(180, 0, 0, 96);
                } else if (tile.maskNumber <= 55000) {
                    color = IM_COL32(220, 0, 0, 96);
                } else {
                    color = IM_COL32(255, 0, 0, 104);
                }

                drawList->AddRectFilled(p0, p1, color);

                // ImGui::SetCursorScreenPos({p0.x + 3.0f, p0.y});
                // ImGui::PushFont(NULL, 10.0f);
                // ImGui::Text("%u", tile.soundType);
                // ImGui::PopFont();

                ImVec2 mousePos = ImGui::GetMousePos();
                if (ImGui::IsWindowFocused() &&
                    ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                    mousePos.x >= p0.x && mousePos.x < p1.x &&
                    mousePos.y >= p0.y && mousePos.y < p1.y)
                {
                    drawList->AddRect(p0, p1, IM_COL32(255, 255, 0, 255));

                    ImGui::SetTooltip("Mask: %u\n"
                                      "Sound: %u\n"
                                      "Type: %u",
                                      tile.maskNumber, tile.soundType, tile.tileType);
                }
            }

            ImVec2 chunkMin = ImVec2(drawPosition.x + correctX, drawPosition.y + correctY);
            ImVec2 chunkMax = ImVec2(chunkMin.x + chunkPixelWidth, chunkMin.y + chunkPixelHeight);

            ImVec2 mousePos = ImGui::GetMousePos();
            if (ImGui::IsWindowFocused() &&
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
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const int tileWidth = 12;
    const int tileHeight = 9;
    for (const SEF_Person& person : level.data().sefData.persons) {
        ImVec2 position;
        position.x = drawPosition.x + person.position.x * tileWidth;
        position.y = drawPosition.y + person.position.y * tileHeight;

        bool fullAlpha = true;
        ImVec2 mousePos = ImGui::GetMousePos();
        if (ImGui::IsWindowFocused() &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            mousePos.x >= position.x && mousePos.x < (position.x + tileWidth) &&
            mousePos.y >= position.y && mousePos.y < (position.y + tileHeight))
        {
            ImGui::SetTooltip("index: %d\n"
                              "tribe: %s\n"
                              "scr_dialog: %s\n"
                              "scr_inv: %s",
                              person.literaryNameIndex,
                              person.tribe.c_str(),
                              person.scriptDialog.c_str(),
                              person.scriptInventory.c_str());
        } else if (ImGui::IsWindowFocused() &&
                   ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            fullAlpha = false;
        }

        drawList->AddRectFilled(position, {position.x + tileWidth, position.y + tileHeight}, IM_COL32(255, 228, 0, fullAlpha ? 192 : 64));
        drawList->AddRect(position, {position.x + tileWidth, position.y + tileHeight}, IM_COL32(0, 0, 0, fullAlpha ? 192 : 64));

        const ImVec2 textPos = {position.x + tileWidth + 2.0f, position.y + 4.0f};
        const ImVec2 textSize = ImGui::CalcTextSize(person.literaryName.c_str());
        drawList->AddRectFilled(textPos, {textPos.x + textSize.x, textPos.y + textSize.y}, IM_COL32(0, 0, 0, fullAlpha ? 164 : 48));

        ImGui::SetCursorScreenPos(textPos);
        ImGui::TextColored(ImVec4(1.0f, 0.95f, 0.0f, fullAlpha ? 1.0f : 0.4f), "%s", person.literaryName.c_str());
    }
}
