#pragma once
#include <vector>
#include <string>

class SdbViewer
{
public:
    SdbViewer() = delete;

    static bool update(bool& showWindow, std::string_view rootDirectory, const std::vector<std::string>& files);
};

