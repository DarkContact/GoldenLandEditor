#pragma once
#include <span>

#include "parsers/CS_Parser.h"
#include "Types.h"

class CsExecutorViewer {
public:
    CsExecutorViewer() = delete;

    static void update(bool& showWindow,
                       bool& needUpdate,
                       std::span<const CS_Node> nodes,
                       const UMapStringVar_t& globalVars);
};

