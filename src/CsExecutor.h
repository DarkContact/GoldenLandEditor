#pragma once
#include <string_view>
#include <vector>
#include <span>
#include <set>

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
    CsExecutor(std::span<const CS_Node> nodes, const StringHashTable<AgeVariable_t>& globalVars);

    static bool readGlobalVariables(std::string_view varsPath, StringHashTable<AgeVariable_t>& globalVars, std::string* error);

    void restart(bool resetAll);

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
    int dialogsAnswersCount() const;

    StringHashTable<AgeVariable_t>& scriptVars();
    ExecuteStatus currentStatus() const;
    const char* currentStatusString() const;

    StringHashTable<int>& heroStats();
    StringHashTable<int>& functionsRetValues();

    bool isNodeExecuted(int index) const;
    float executedPercent() const;

private:
    void readScriptVariables();
    void initHeroStats();
    void initFunctionsRetValues();

    bool logicalOpcode(const CS_Node& node);
    bool compareOpcode(const CS_Node& node);
    AgeVariable_t arithmeticOpcode(const CS_Node& node);
    AgeVariable_t applyBinaryOp(const AgeVariable_t& lhs, const AgeVariable_t& rhs, int opcode);
    int funcOpcode(const CS_Node& node);

    void fatalError(const std::string& message) const;

    int RS_GetPersonParameterI(std::string_view person, std::string_view param);
    int RS_IsPersonExistsI(std::string_view levelName, std::string_view npcTechName);
    int RS_GetDialogEnabled(int param);
    int RS_GetMoney();
    int RS_GetItemCountI(std::string_view npcTechName, std::string_view itemTechName);
    int RS_TestPersonHasItem(std::string_view npcTechName, std::string_view itemTechName);
    int D_CloseDialog(int param);

    std::span<const CS_Node> m_nodes;
    StringHashTable<AgeVariable_t> m_globalVars;
    StringHashTable<AgeVariable_t> m_scriptVars;
    std::vector<CS_Node> m_funcs;
    std::vector<CS_Node> m_dialogFuncs;

    StringHashTable<int> m_heroStats;
    StringHashTable<int> m_functionsRetValues;
    std::set<int> m_executedNodeIndexes;

    int m_currentNodeIndex = 0;
    ExecuteStatus m_currentStatus = kStart;
    static constexpr int kStopCounter = 10000;
    int m_counter = 0;
};
