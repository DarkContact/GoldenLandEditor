#pragma once
#include <string_view>
#include <vector>
#include <string>

struct CS_Node;
struct SDB_Data;

class CsViewer {
public:
    CsViewer() = delete;

    static void update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles);

    static std::string csNodeString(const CS_Node& node, const SDB_Data& sdbDialogs, bool showDialogPhrases = true);
};

