#pragma once
#include <string_view>
#include <vector>
#include <span>

#include "parsers/CS_Parser.h"
#include "Types.h"

enum VarType {
    kInt = 0,
    kDword,
    kFloat,
    kString
};

class CsExecutor {
public:
    CsExecutor(std::span<const CS_Node> nodes, const UMapStringVar_t& globalVars);

    static bool readGlobalVariables(std::string_view varsPath, UMapStringVar_t& globalVars, std::string* error);

    void restart();
    bool next();

    int currentNodeIndex() const;
    int counter() const;
    std::vector<std::string> variablesInfo() const;
    std::vector<std::string> funcsInfo() const;

private:
    void readScriptVariables();

    std::span<const CS_Node> m_nodes;
    UMapStringVar_t m_globalVars;
    UMapStringVar_t m_scriptVars;

    std::vector<std::string> m_funcs;
    int m_currentNodeIndex = 0;

    int m_counter = 0;
    static constexpr int kStopCounter = 10000;
};
