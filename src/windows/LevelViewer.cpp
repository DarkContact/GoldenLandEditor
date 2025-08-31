#include "LevelViewer.h"

#include "imgui.h"
#include "imgui_internal.h"

bool LevelViewer::update(bool& showWindow, Level& level)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(level.data().name.c_str(), &showWindow, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Minimap", NULL, level.data().imgui.showMinimap)) {
                level.data().imgui.showMinimap = !level.data().imgui.showMinimap;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)level.data().background, ImVec2((float)level.data().background->w, (float)level.data().background->h));

    if (level.data().minimap && level.data().imgui.showMinimap)
    {
        updateMinimap(level);
    }

    // ImGui::SetCursorScreenPos(pos);
    // ImGui::Text("%dx%d", level.data().background->w, level.data().background->h);

    ImGui::End();
    return true;
}

void LevelViewer::updateMinimap(Level& level)
{
    const ImVec2 minimapSize = ImVec2(level.data().minimap->w, level.data().minimap->h);
    ImVec2 minimapPosition;

    // Миникарта и белая обводка
    {
        ImGuiStyle& style = ImGui::GetStyle();

        float menu_bar_height = ImGui::GetCurrentWindow()->MenuBarHeight;
        float scrollbar_width = (ImGui::GetCurrentWindow()->ScrollbarY ? style.ScrollbarSize : 0.0f);

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        minimapPosition = ImVec2(windowPos.x + windowSize.x - minimapSize.x - scrollbar_width - 8.0f,
                                 windowPos.y + menu_bar_height + 26.0f);
        ImGui::SetCursorScreenPos(minimapPosition);

        ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

        ImGui::Image((ImTextureID)level.data().minimap, minimapSize);

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
        ImRect minimapRect(minimapPosition, ImVec2(minimapPosition.x + minimapSize.x, minimapPosition.y + minimapSize.y));

        auto& imgui = level.data().imgui;

        bool hoveredView = viewRect.Contains(io.MousePos);
        bool hoveredMinimap = minimapRect.Contains(io.MousePos);

        // Начало перетаскивания рамки
        if (!imgui.draggingMinimap && hoveredView && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            imgui.draggingMinimap = true;
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

            ImGui::SetScrollX(centerX);
            ImGui::SetScrollY(centerY);
        }

        // Обновим рамку
        viewTopLeft = ImVec2(minimapPosition.x + ImGui::GetScrollX() * scaleX,
                             minimapPosition.y + ImGui::GetScrollY() * scaleY);
        viewBottomRight = ImVec2(viewTopLeft.x + viewSize.x * scaleX,
                                 viewTopLeft.y + viewSize.y * scaleY);

        // Рисуем рамку
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(viewTopLeft, viewBottomRight, IM_COL32(255, 255, 0, 220), 0.0f, 0, 2.0f);
    }
}
