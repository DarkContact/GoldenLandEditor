#pragma once
#include <vector>
#include <string>

class CsViewer {
public:
    CsViewer() = delete;

    static void update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& csFiles);
};

