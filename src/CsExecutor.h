#pragma once
#include <unordered_map>
#include <string_view>
#include <variant>
#include <cstdint>
#include <span>

#include "parsers/CS_Parser.h"
#include "Types.h"

enum VarType {
    kInt = 0,
    kDword,
    kFloat,
    kString
};

typedef std::variant<int32_t, uint32_t, double, std::string> Variable_t;

class CsExecutor {
public:
    CsExecutor(std::span<const CS_Node> nodes);

    bool readGlobalVariables(std::string_view rootDirectory, std::string* error);
    bool readScriptVariables(std::string* error);

    bool next();

    std::vector<std::string> variablesInfo();

private:
    std::span<const CS_Node> m_nodes;
    std::unordered_map<std::string, Variable_t, StringViewHash, StringViewEqual> m_globalVars;
    std::unordered_map<std::string, Variable_t, StringViewHash, StringViewEqual> m_scriptVars;

    int m_currentNodeIndex = 0;
};
