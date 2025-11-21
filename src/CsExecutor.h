#pragma once
#include <unordered_map>
#include <string_view>
#include <variant>
#include <cstdint>
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
    CsExecutor(std::span<const CS_Node> nodes);

    bool readGlobalVariables(std::string_view rootDirectory, std::string* error);
    bool readScriptVariables(std::string* error);

    void restart();
    bool next();

    int currentNodeIndex() const;
    int counter() const;
    std::vector<std::string> variablesInfo() const;
    std::vector<std::string> funcsInfo() const;

private:
    std::span<const CS_Node> m_nodes;
    std::unordered_map<std::string, Variable_t, StringViewHash, StringViewEqual> m_globalVars;
    std::unordered_map<std::string, Variable_t, StringViewHash, StringViewEqual> m_scriptVars;

    std::vector<std::string> m_funcs;
    int m_currentNodeIndex = 0;

    int m_counter = 0;
    static constexpr int kStopCounter = 10000;
};
