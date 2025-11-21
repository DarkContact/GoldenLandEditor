#pragma once
#include <string_view>
#include <span>

#include "parsers/CS_Parser.h"

class CsExecutorViewer {
public:
    CsExecutorViewer() = delete;

    static void update(bool& showWindow, bool& needUpdate, std::string_view rootDirectory, std::span<const CS_Node> nodes);
};

