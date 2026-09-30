#pragma once
#include <vector>
#include <string>

#include "imgui.h"

#include "parsers/SDB_Parser.h"

struct SDL_Renderer;

class SdbViewer {
public:
    SdbViewer();

    void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& files);

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
    bool m_showFormattedSymbols = true;
    bool m_filterNeedsUpdate = false;

    bool m_showAddRecordWindow = false;
    int m_addRecordId = 0;
    std::string m_addRecordText;
};
