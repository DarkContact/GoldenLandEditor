#include "LevelViewer.h"

#include "imgui.h"
#include "imgui_internal.h"

bool LevelViewer::update(bool& showWindow, Level& level)
{
    ImGuiStyle& style = ImGui::GetStyle();

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

    // ImGui::SetCursorScreenPos(pos);
    // ImGui::Text("%dx%d", level.data().background->w, level.data().background->h);

    if (level.data().minimap && level.data().imgui.showMinimap)
    {
        float menu_bar_height = ImGui::GetCurrentWindow()->MenuBarHeight;
        float scrollbar_width = (ImGui::GetCurrentWindow()->ScrollbarY ? style.ScrollbarSize : 0.0f);

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 imageSize = ImVec2(level.data().minimap->w, level.data().minimap->h);

        ImVec2 imagePos = ImVec2(windowPos.x + windowSize.x - imageSize.x - scrollbar_width - 8.0f,
                                 windowPos.y + menu_bar_height + 26.0f);
        ImGui::SetCursorScreenPos(imagePos);

        ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));

        ImGui::Image((ImTextureID)level.data().minimap, imageSize);

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        // Рисуем рамку области видимости
        float mapWidth = (float)level.data().background->w;
        float mapHeight = (float)level.data().background->h;

        ImVec2 contentMin = ImGui::GetCurrentWindow()->ContentRegionRect.Min;
        ImVec2 contentMax = ImGui::GetCurrentWindow()->ContentRegionRect.Max;
        ImVec2 viewSize = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

        float scaleX = imageSize.x / mapWidth;
        float scaleY = imageSize.y / mapHeight;

        ImVec2 viewTopLeft = ImVec2(imagePos.x + ImGui::GetScrollX() * scaleX, imagePos.y + ImGui::GetScrollY() * scaleY);
        ImVec2 viewBottomRight = ImVec2(viewTopLeft.x + viewSize.x * scaleX, viewTopLeft.y + viewSize.y * scaleY);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(viewTopLeft, viewBottomRight, IM_COL32(255, 255, 0, 220), 0.0f, 0, 2.0f);
    }

    ImGui::End();
    return true;
}
