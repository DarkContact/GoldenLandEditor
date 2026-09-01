#include "PadViewer.h"

#include <algorithm>
#include <format>

#include "utils/TracyProfiler.h"
#include "utils/IoUtils.h"

PadViewer::PadViewer() {}

void PadViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& padFiles)
{
    Tracy_ZoneScoped;
    using namespace IoUtils;

    if (showWindow && !padFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("PAD Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(400, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            m_textFilter.Draw();
            ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(padFiles.size()); ++i)
                {
                    if (m_textFilter.PassFilter(padFiles[i].c_str())
                        && ImGui::Selectable(padFiles[i].c_str(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        m_padData = PAD_Parser::parse(std::format("{}/{}", rootDirectory, padFiles[i]), &m_error);

                        needResetScroll = true;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        if (!padFiles.empty() && m_padData) {
            ImGui::BeginGroup();

                ImGui::BeginChild("item view", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);

                ImGui::Text("Size: %zu, Masks: %u", m_padData->animationData.size(), m_padData->animationMasks);
                for (auto animation : m_padData->animationTypes) {
                    ImGui::Text("%s", animationTypeMaskToString(animation).data());
                }

                ImGui::Separator();

                for (size_t offset = 0; offset < m_padData->animationData.size();) {
                    size_t startOffset = offset;
                    uint32_t value = readUInt32(m_padData->animationData, offset);

                    offset = startOffset;
                    uint16_t value16L = readUInt16(m_padData->animationData, offset);
                    uint16_t value16R = readUInt16(m_padData->animationData, offset);

                    bool isHighlight = std::ranges::any_of(m_padData->animationTypes, [value](uint32_t x) { return x == value; });

                    const ImGuiStyle& style = ImGui::GetStyle();
                    ImVec4 textColor = isHighlight ? ImVec4(1.0f, 0.95f, 0.0f, 1.0f) : style.Colors[ImGuiCol_Text];
                    if (value > std::numeric_limits<uint16_t>::max()) {
                        ImGui::TextColored(textColor, "[i:%zu] %u, %u", (offset / 4) - 1, value16L, value16R);
                    } else {
                        ImGui::TextColored(textColor, "[i:%zu] %u", (offset / 4) - 1, value);
                    }
                }

                ImGui::Separator();

                ImGui::Text("%d", m_padData->endSize);
                for (size_t offset = 0; offset < m_padData->endData.size();) {
                    uint32_t value = readUInt32(m_padData->endData, offset);
                    ImGui::Text("[i:%zu] %u", (offset / 4) - 1, value);
                }

                ImGui::EndChild();

            ImGui::EndGroup();
        } else if (m_selectedIndex >= 0) {
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_error.c_str());
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_onceWhenClose = true;
        m_textFilter.Clear();
        m_error.clear();
        m_padData = {};
    }
}

bool PadViewer::isAnimating() const {
    return true;
}
