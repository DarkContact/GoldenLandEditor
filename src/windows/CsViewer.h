#pragma once
#include <string_view>
#include <vector>
#include <string>

#include "imgui.h"

#include "parsers/CS_Parser.h"
#include "parsers/SDB_Parser.h"
#include "windows/CsExecutorViewer.h"
#include "Types.h"

class CsViewer {
public:
    CsViewer();

    void update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles);
    void injectPlaySoundAndGeneratePhrases(std::string_view saveRootDirectory, std::string_view rootDirectory, const std::vector<std::string>& csFiles);

private:
    void injectPlaySoundFunc(size_t insertPos, std::string_view soundFile);

    int m_selectedIndex = -1;
    ImGuiTextFilter m_textFilterFile;
    ImGuiTextFilter m_textFilterString;
    CS_Data m_csData;
    SDB_Data m_sdbDialogs;
    UMapStringVar_t m_globalVars;
    std::string m_csError;
    std::vector<bool> m_funcNodes;
    bool m_showOnlyFunctions = false;
    bool m_showDialogPhrases = true;
    bool m_showExecuteWindow = false;

    bool m_onceWhenOpen = false;
    bool m_onceWhenClose = true;

    const ImVec4 m_funcTextColor = ImVec4(1.0f, 0.92f, 0.5f, 1.0f); // TODO: Нечитаемо в светлой теме
    const ImVec4 m_execTextColor = ImVec4(0.0f, 0.85f, 0.0f, 1.0f);

    CsExecutorViewer m_csExecutorViewer;
};

