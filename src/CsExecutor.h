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

    enum ExecuteStatus {
        kStart,
        kRestart,
        kContinue,
        kWaitUser,
        kEnd,
        kInfinity
    };
    bool next();
    void userInput(uint8_t value);

    int currentNodeIndex() const;
    int counter() const;
    std::vector<std::string> variablesInfo() const;
    std::vector<std::string> funcsInfo() const;
    std::array<int, 11> dialogsData() const;

    UMapStringVar_t& scriptVars();
    ExecuteStatus currentStatus() const;

private:
    void readScriptVariables();

    bool compareOpcode(const CS_Node& node);
    int funcOpcode(const CS_Node& node);

    std::span<const CS_Node> m_nodes;
    UMapStringVar_t m_globalVars;
    UMapStringVar_t m_scriptVars;
    std::vector<CS_Node> m_funcs;
    std::vector<CS_Node> m_dialogFuncs;

    int m_currentNodeIndex = 0;
    ExecuteStatus m_currentStatus = kStart;

    int m_counter = 0;
    static constexpr int kStopCounter = 10000;
};
