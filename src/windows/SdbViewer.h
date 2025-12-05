#pragma once
#include <vector>
#include <string>

#include "imgui.h"

#include "parsers/SDB_Parser.h"

class SdbViewer
{
public:
    SdbViewer();

    void update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files);

private:
    enum SearchByType {
        kId,
        kText
    };

    int m_selectedIndex = -1;
    SDB_Data m_sdbRecords;
    ImGuiTextFilter m_textFilterFile;
    ImGuiTextFilter m_textFilterString;
    SearchByType m_searchByType = kText;
    std::vector<int> m_filteredKeys;
    bool m_sameHeightForRow = true;
    bool m_onceWhenClose = true;
    bool m_showFormattedSymbols = false;
    bool m_filterNeedsUpdate = false;
};
