#include "SdbViewer.h"

#include <format>

#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

SdbViewer::SdbViewer() {

}

size_t makeVisibleSymbols(std::string_view sv, std::span<char> out) {
    size_t writePos = 0;
    const size_t maxWrite = out.size() ? out.size() - 1 : 0;

    auto appendChar = [&](char c) {
        if (writePos < maxWrite)
            out[writePos] = c;
        ++writePos;
    };

    auto appendEscaped = [&](char c, char esc) {
        appendChar('\\');
        appendChar(esc);
    };

    for (char c : sv) {
        switch (c) {
            case '\n': appendEscaped(c, 'n'); break;
            case '\r': appendEscaped(c, 'r'); break;
            default:   appendChar(c);         break;
        }
    }

    if (!out.empty()) {
        out[std::min(writePos, maxWrite)] = '\0';
    }

    return writePos;
}

void SdbViewer::update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files)
{
    Tracy_ZoneScoped;
    
    if (showWindow && !files.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;
        bool needUpdateFilter = false;

        if (m_filterNeedsUpdate) {
            needUpdateFilter = true;
            m_filterNeedsUpdate = false;
        }

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("SDB Viewer", &showWindow);

        ImGui::BeginChild("left pane", ImVec2(340, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        m_textFilterFile.Draw();
        ImGui::Separator();
            ImGui::BeginChild("file list");
            for (int i = 0; i < static_cast<int>(files.size()); ++i)
            {
                if (m_textFilterFile.PassFilter(files[i].c_str())
                    && ImGui::Selectable(files[i].c_str(), m_selectedIndex == i))
                {
                    m_selectedIndex = i;

                    m_sdbRecords.strings.clear();
                    m_filteredKeys.clear();

                    std::string error;
                    if (!SDB_Parser::parse(std::format("{}/{}", rootDirectory, files[i]), m_sdbRecords, &error)) {
                        LogFmt("SdbViewer error: {}", error);
                    }

                    m_filteredKeys.reserve(m_sdbRecords.strings.size());
                    m_sameHeightForRow = true;
                    for (const auto& [id, text] : m_sdbRecords.strings) {
                        m_filteredKeys.push_back(id);

                        if (m_sameHeightForRow) {
                            size_t pos = text.find_first_of("\n\r");
                            if (pos != std::string::npos) {
                                m_sameHeightForRow = false;
                            }
                        }
                    }

                    needResetScroll = true;
                    needUpdateFilter = true;
                }
            }
            ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
            if (ImGui::RadioButton("ID", m_searchByType == kId)) {
                m_searchByType = kId;
                needUpdateFilter = true;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Text", m_searchByType == kText)) {
                m_searchByType = kText;
                needUpdateFilter = true;
            }
            ImGui::SameLine();
            if (m_textFilterString.Draw()) {
                needUpdateFilter = true;
            }

            if (!m_sdbRecords.strings.empty()) {
                if (ImGui::BeginTable("content", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {
                    if (needResetScroll) {
                        ImGui::SetScrollX(0.0f);
                        ImGui::SetScrollY(0.0f);
                    }

                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("ID");
                    ImGui::TableSetupColumn("Текст");
                    ImGui::TableHeadersRow();

                    int counter = 0;
                    constexpr size_t kVisibleTextSize = 16384;
                    char visibleText[kVisibleTextSize];
                    if (needUpdateFilter) {
                        m_filteredKeys.clear();
                        char idStr[16];
                        for (const auto& [id, text] : m_sdbRecords.strings) {
                            std::string_view filter;
                            if (m_searchByType == kText) {
                                if (m_showFormattedSymbols) {
                                    size_t visibleTextSize = makeVisibleSymbols(text, visibleText);
                                    assert(visibleTextSize <= kVisibleTextSize);
                                    filter = {visibleText, std::min(kVisibleTextSize, visibleTextSize)};
                                } else {
                                    filter = std::string_view{text.data(), text.size()};
                                }
                            } else if (m_searchByType == kId) {
                                auto idSize = StringUtils::formatToBuffer(idStr, "{}", id);
                                filter = std::string_view{idStr, idSize};
                            }
                            bool pass = m_textFilterString.PassFilter(filter.data(), filter.data() + filter.size());
                            if (pass) {
                                m_filteredKeys.push_back(id);
                            }
                            ++counter;
                        }
                    }

                    auto tableFormat = [this, &visibleText, kVisibleTextSize] (int id) {
                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        ImGui::Text("%d", id);

                        std::string_view text = m_sdbRecords.strings[id];
                        if (m_showFormattedSymbols) {
                            size_t visibleTextSize = makeVisibleSymbols(text, visibleText);
                            assert(visibleTextSize <= kVisibleTextSize);
                            text = {visibleText, std::min(kVisibleTextSize, visibleTextSize)};
                        }
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(text.data(), text.data() + text.size());

                        if (ImGui::IsItemHovered()) { // TODO: Доработать область наведения на всю строку
                            ImVec4 hoverColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
                            for (int col = 0; col < 2; ++col) {
                                ImGui::TableSetColumnIndex(col);
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(hoverColor));
                            }

                            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) { // TODO: Показывать тултип только когда текст действительно не вмещается на экран
                                ImGui::BeginTooltip();

                                std::string_view fullText = m_sdbRecords.strings[id];
                                bool longText = fullText.size() > 6000;

                                ImGui::PushTextWrapPos(longText ? 800.0f : 600.0f);
                                ImGui::PushFont(NULL, longText ? (ImGui::GetStyle().FontSizeBase - 1.0f)
                                                               : 0.0f);
                                ImGui::TextUnformatted(fullText.data(), fullText.data() + fullText.size());
                                ImGui::PopFont();
                                ImGui::PopTextWrapPos();

                                ImGui::EndTooltip();
                            }
                        }

                        ImGui::PushID(id);
                        if (ImGui::BeginPopupContextItem("text context menu")) {
                            if (ImGui::MenuItem("Copy")) {
                                ImGui::SetClipboardText(text.data());
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                    };

                    bool useClipping = m_sameHeightForRow || m_showFormattedSymbols;
                    if (useClipping) {
                        ImGuiListClipper clipper;
                        clipper.Begin((int)m_filteredKeys.size());
                        while (clipper.Step()) {
                            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                                int id = m_filteredKeys[i];
                                tableFormat(id);
                            }
                        }
                    } else {
                        for (int id : m_filteredKeys) {
                            tableFormat(id);
                        }
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();

            if (ImGui::Checkbox("Show formatted symbols", &m_showFormattedSymbols)) {
                m_filterNeedsUpdate = true;
            }
        }
        ImGui::EndGroup();

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_sdbRecords.strings.clear();
        m_textFilterFile.Clear();
        m_textFilterString.Clear();
        m_filteredKeys.clear();
        m_onceWhenClose = true;
    }
}
