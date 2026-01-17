#include "CsExecutor.h"

#include <algorithm>
#include <exception>
#include <cassert>
#include <format>

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"

#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"

CsExecutor::CsExecutor(std::span<const CS_Node> nodes, const UMapStringVar_t& globalVars) :
    m_nodes(nodes),
    m_globalVars(globalVars)
{
    assert(!m_nodes.empty());
    assert(!m_globalVars.empty());

    readScriptVariables();
}

bool parse3(std::string_view line,
            std::string_view& a,
            std::string_view& b,
            std::string_view& c)
{
    size_t i = 0;

    auto skip_ws = [line](size_t& pos) {
        pos = line.find_first_not_of(" \t", pos);
    };

    auto read_token = [line](size_t& pos, std::string_view& out) {
        if (pos == std::string_view::npos) return false;
        size_t start = pos;
        pos = line.find_first_of(" \t", pos);
        out = line.substr(start, pos - start);
        return true;
    };

    skip_ws(i);
    if (!read_token(i, a)) return false;

    skip_ws(i);
    if (!read_token(i, b)) return false;

    skip_ws(i);
    if (!read_token(i, c)) return false;

    return true;
}

bool CsExecutor::readGlobalVariables(std::string_view varsPath, UMapStringVar_t& globalVars, std::string* error)
{
    auto fileData = FileUtils::loadFile(varsPath, error);
    if (fileData.empty()) {
        return false;
    }

    std::string_view fileStringView((char*)fileData.data(), fileData.size());
    StringUtils::forEachLine(fileStringView, [&globalVars] (std::string_view line)
    {
        line = StringUtils::eraseOneLineComment(line);
        line = StringUtils::trim(line);
        if (line.empty()) return;

        std::string_view type, name, value;
        if (!parse3(line, type, name, value)) {
            LogFmt("Can't parse line: {}", line);
            return;
        }

        Variable_t varValue;
        if (type == "int") {
            varValue = StringUtils::toInt(value);
        } else if (type == "DWORD") {
            varValue = StringUtils::toUInt(value);
        } else if (type == "float") {
            varValue = StringUtils::toDouble(value);
        } else if (type == "string") {
            varValue = std::string(StringUtils::extractQuotedValue(value));
        }

        globalVars.emplace(std::string(name), varValue);
    });

    return true;
}

void CsExecutor::readScriptVariables()
{
    m_scriptVars.clear();
    for (const auto& node : m_nodes) {
        if (node.opcode == OpcodeType::kStringVarName) {
            if (auto it = m_globalVars.find(node.text); it != m_globalVars.end()) {
                m_scriptVars.emplace(node.text, it->second);
            } else {
                LogFmt("Script variable '{}' not found in globals vars", node.text);
            }
        }
    }
}

void CsExecutor::restart(bool resetAll)
{
    m_counter = 0;
    m_currentNodeIndex = 0;
    m_currentStatus = kStart;
    m_funcs.clear();
    m_dialogFuncs.clear();

    if (resetAll) {
        readScriptVariables();
    } else {
        // Необходимо для корректной работы диалогов
        m_scriptVars["LastPhrase"] = 0u;
        m_scriptVars["LastAnswer"] = 0u;
    }
}

int CsExecutor::currentNodeIndex() const {
    return m_currentNodeIndex;
}

int CsExecutor::counter() const {
    return m_counter;
}

std::vector<std::string> CsExecutor::variablesInfo() const {
    std::vector<std::string> out;
    out.reserve(m_scriptVars.size());

    for (auto& [name, val] : m_scriptVars)
    {
        if (auto p = std::get_if<int32_t>(&val))
            out.emplace_back("int " + name + " " + std::to_string(*p));
        else if (auto p = std::get_if<uint32_t>(&val))
            out.emplace_back("DWORD " + name + " " + std::to_string(*p));
        else if (auto p = std::get_if<double>(&val))
            out.emplace_back("float " + name + " " + std::to_string(*p));
        else if (auto p = std::get_if<std::string>(&val))
            out.emplace_back("string " + name + " " + *p);
    }
    return out;
}

std::vector<std::string> CsExecutor::funcsInfo() const {
    std::vector<std::string> out;
    out.reserve(m_funcs.size());
    char buffer[768];
    for (const auto& func : m_funcs) {
        constexpr int kArgsInfoSize = 512;
        char argsInfo[kArgsInfoSize];
        argsInfo[0] = '\0';
        size_t offset = 0;
        for (int j = 0; j < func.args.size(); j++) {
            int32_t idx = func.args[j];
            if (idx == -1) break;

            const CS_Node node = m_nodes[idx];
            if (node.opcode == kStringLiteral) {
                char varLiteral[320];
                StringUtils::decodeWin1251ToUtf8Buffer(node.text, varLiteral);
                offset += StringUtils::formatToBuffer(std::span<char>(argsInfo + offset, kArgsInfoSize - offset), "\"{}\", ", varLiteral);
            } else if (node.opcode == kNumberLiteral) {
                offset += StringUtils::formatToBuffer(std::span<char>(argsInfo + offset, kArgsInfoSize - offset), "{}, ", node.value);
            } else if (node.opcode == kStringVarName) {
                offset += StringUtils::formatToBuffer(std::span<char>(argsInfo + offset, kArgsInfoSize - offset), "{}, ", m_scriptVars.at(node.text));
            } else {
                fatalError( std::format("[currentNode.opcode: {}]", csOpcodeToString(node.opcode)) );
            }
        }

        auto argsInfoTrimmed = StringUtils::trimRight(argsInfo);
        if (argsInfoTrimmed.ends_with(',')) {
            argsInfoTrimmed.remove_suffix(1);
        }
        size_t bufferSize = StringUtils::formatToBuffer(buffer, "{}({})", csFuncToString(func.value), argsInfoTrimmed);
        out.emplace_back(std::string{buffer, bufferSize});
    }
    return out;
}

std::array<int, 11> CsExecutor::dialogsData() const
{
    if (m_currentStatus != kWaitUser || m_dialogFuncs.empty()) return {-1};
    assert(m_dialogFuncs.size() >= 2 && m_dialogFuncs.size() <= 11);

    std::array<int, 11> result;
    result.fill(-1);

    const CS_Node& sayNode = m_dialogFuncs.front();
    result[0] = m_nodes[sayNode.args.front()].value;
    for (size_t i = 1; i < m_dialogFuncs.size(); ++i) {
        const CS_Node& answerNode = m_dialogFuncs[i];
        result[i] = m_nodes[answerNode.args.front()].value;
    }
    return result;
}


int CsExecutor::dialogsAnswerCount() const {
    return m_dialogFuncs.size() - 1;
}

bool CsExecutor::next()
{
    // Защита от бесконечного выполнения
    if (m_counter >= CsExecutor::kStopCounter) {
        m_currentStatus = kInfinity;
        return false;
    }
    ++m_counter;

    const CS_Node& currentNode = m_nodes[m_currentNodeIndex];
    m_executedNodeIndexes.insert(m_currentNodeIndex);

    OpcodeGroup group = csOpcodeToGroup(currentNode.opcode);
    if (group == kLogical) {
        bool isTrue = logicalOpcode(currentNode);
        m_currentNodeIndex = isTrue ? currentNode.c : currentNode.d;
    } else if (group == kComparison) {
        bool isTrue = compareOpcode(currentNode);
        m_currentNodeIndex = isTrue ? currentNode.c : currentNode.d;
    } else {

        if (currentNode.opcode == kJmp) {
            if (currentNode.d == -1) {
                bool exists = std::any_of(m_dialogFuncs.cbegin(), m_dialogFuncs.cend(), [](const CS_Node& node) { return (uint32_t)node.value == kD_Say; });
                m_currentStatus = exists ? kWaitUser : kEnd;
                return false;
            }

            m_currentNodeIndex = currentNode.c;
        } else if (currentNode.opcode == kAssign) {
            const CS_Node& rNode = m_nodes[currentNode.b];
            Variable_t rValue;
            if (rNode.opcode == kStringVarName) {
                rValue = m_scriptVars[rNode.text];
            } else if (rNode.opcode == kNumberLiteral) {
                rValue = (int)rNode.value; // TODO: Корректное приведение типов
            } else if (rNode.opcode == kFunc) {
                rValue = funcOpcode(rNode); // TODO: D_CloseDialog должен прерывать выполнение (m_currentStatus = kEnd;)
            } else {
                fatalError( std::format("[currentNode.opcode: assign] [rNode.opcode: {}]", csOpcodeToString(rNode.opcode)) );
            }

            const CS_Node& lNode = m_nodes[currentNode.a];
            if (lNode.opcode == kStringVarName) {
                m_scriptVars[lNode.text] = rValue;
            } else {
                fatalError( std::format("[currentNode.opcode: assign] [lNode.opcode: {}]", csOpcodeToString(lNode.opcode)) );
            }

            //LogFmt("[currentNode.opcode: assign] lNode.opcode: {}, rNode.opcode: {}", csOpcodeToString(lNode.opcode), csOpcodeToString(rNode.opcode));
            m_executedNodeIndexes.insert(currentNode.a);
            m_executedNodeIndexes.insert(currentNode.b);
            m_currentNodeIndex = currentNode.d;
        } else {
            std::string errorMessage;
            if (currentNode.a == -1 || currentNode.b == -1) {
                errorMessage = std::format("[currentNode.opcode: {}]", csOpcodeToString(currentNode.opcode));
            } else {
                const CS_Node& lNode = m_nodes[currentNode.a];
                const CS_Node& rNode = m_nodes[currentNode.b];
                errorMessage = std::format("[currentNode.opcode: {}] lNode.opcode: {}, rNode.opcode: {}",
                                           csOpcodeToString(currentNode.opcode), csOpcodeToString(lNode.opcode), csOpcodeToString(rNode.opcode));
            }
            fatalError(errorMessage);
        }

    }

    m_currentStatus = kContinue;
    return true;
}

void CsExecutor::userInput(uint8_t answer) {
    assert(!m_dialogFuncs.empty());
    CS_Node sayNode = m_dialogFuncs[0];
    assert((uint32_t)sayNode.value == kD_Say);

    // Мы застряли на вводе, т.к. ответить юзеру нечем
    // Пример: l91.p216_fisherman_hermit.d216.age.cs (goldland2)
    if (answer >= m_dialogFuncs.size()) {
        Log("[WARNING] UserInput without answers!");
        m_currentStatus = kWaitUser;
        return;
    }

    CS_Node answerNode = m_dialogFuncs[answer];
    assert((uint32_t)answerNode.value == kD_Answer);

    m_scriptVars["LastPhrase"] = (uint32_t)m_nodes[sayNode.args.front()].value;
    m_scriptVars["LastAnswer"] = (uint32_t)m_nodes[answerNode.args.front()].value;

    m_dialogFuncs.clear();

    m_currentStatus = kRestart;
    m_currentNodeIndex = 0;
}

bool CsExecutor::logicalOpcode(const CS_Node& node) {
    const CS_Node& lNode = m_nodes[node.a];
    const CS_Node& rNode = m_nodes[node.b];
    int isLeftValue = 0;
    int isRightValue = 0;
    if (csOpcodeToGroup(lNode.opcode) == kComparison) {
        isLeftValue = compareOpcode(lNode);
    } else if (csOpcodeToGroup(lNode.opcode) == kLogical) {
        isLeftValue = logicalOpcode(lNode);
    } else if (lNode.opcode == kFunc) {
        isLeftValue = funcOpcode(lNode);
    } else {
        fatalError( std::format("logicalOpcode [lNode.opcode: {}]", csOpcodeToString(lNode.opcode)) );
    }

    if (csOpcodeToGroup(rNode.opcode) == kComparison) {
        isRightValue = compareOpcode(rNode);
    } else if (csOpcodeToGroup(rNode.opcode) == kLogical) {
        isRightValue = logicalOpcode(rNode);
    } else if (rNode.opcode == kFunc) {
        isRightValue = funcOpcode(rNode);
    } else {
        fatalError( std::format("logicalOpcode [rNode.opcode: {}]", csOpcodeToString(rNode.opcode)) );
    }

    bool isTrue = false;
    if (node.opcode == kLogicOr) {
        isTrue = isLeftValue || isRightValue;
    } else if (node.opcode == kLogicAnd) {
        isTrue = isLeftValue && isRightValue;
    } else {
        fatalError( std::format("logicalOpcode [node.opcode: {}]", csOpcodeToString(node.opcode)) );
    }
    // LogFmt("[currentNode.opcode: {}] lNode.opcode: {}, rNode.opcode: {}", csOpcodeToString(node.opcode), csOpcodeToString(lNode.opcode), csOpcodeToString(rNode.opcode));
    // LogFmt("isLeftValue: {}, isRightValue: {}, isTrue: {}", isLeftValue, isRightValue, isTrue);
    m_executedNodeIndexes.insert(node.a);
    m_executedNodeIndexes.insert(node.b);
    return isTrue;
}

bool CsExecutor::compareOpcode(const CS_Node& node) {
    const CS_Node& lNode = m_nodes[node.a];
    Variable_t lValue;
    if (lNode.opcode == kStringVarName) {
        lValue = m_scriptVars[lNode.text];
    } else if (lNode.opcode == kNumberLiteral) {
        lValue = lNode.value;
    } else if (lNode.opcode == kNumberVarName) {
        lValue = lNode.value; // Корректно?
    } else {
        fatalError( std::format("compareOpcode [lNode.opcode: {}]", csOpcodeToString(lNode.opcode)) );
    }

    const CS_Node& rNode = m_nodes[node.b];
    Variable_t rValue;
    if (rNode.opcode == kStringVarName) {
        rValue = m_scriptVars[rNode.text];
    } else if (rNode.opcode == kNumberLiteral) {
        // NOTE: Приведение типа равного lValue
        std::visit([&rValue, rNode](auto&& lv){
            using T = std::decay_t<decltype(lv)>;
            if constexpr (std::is_same_v<T, std::string>) {
                rValue = rNode.value;
            } else {
                rValue = (T)rNode.value;
            }
        }, lValue);
    } else {
        fatalError( std::format("compareOpcode [rNode.opcode: {}]", csOpcodeToString(rNode.opcode)) );
    }

    bool isTrue = false;
    if (node.opcode == 6) {
        isTrue = (lValue != rValue);
    } else if (node.opcode == 7) {
        isTrue = (lValue == rValue);
    } else if (node.opcode == 8) {
        isTrue = (lValue >= rValue);
    } else if (node.opcode == 9) {
        isTrue = (lValue <= rValue);
    } else if (node.opcode == 10) {
        isTrue = (lValue > rValue);
    } else if (node.opcode == 11) {
        isTrue = (lValue < rValue);
    }
    // LogFmt("[currentNode.opcode: {}] lNode.opcode: {}, rNode.opcode: {}", csOpcodeToString(node.opcode), csOpcodeToString(lNode.opcode), csOpcodeToString(rNode.opcode));
    // LogFmt("lValue: {}, rValue: {}, isTrue: {}", lValue, rValue, isTrue);
    m_executedNodeIndexes.insert(node.a);
    m_executedNodeIndexes.insert(node.b);
    return isTrue;
}

int CsExecutor::funcOpcode(const CS_Node& node)
{
    m_funcs.emplace_back(node);
    uint32_t funcValue = static_cast<uint32_t>(node.value);
    if (funcValue == kD_Say || funcValue == kD_Answer) {
        m_dialogFuncs.emplace_back(node);
    }

    switch (funcValue) {
        case kRS_GetPersonParameterI: {
            std::string_view arg0 = m_nodes[node.args[0]].text;
            std::string_view arg1 = m_nodes[node.args[1]].text;
            return RS_GetPersonParameterI(arg0, arg1);
        }

        case kD_CloseDialog: {
            int arg0 = m_nodes[node.args[0]].value;
            return D_CloseDialog(arg0);
        }
    }

    for (auto arg : node.args) {
        if (arg == -1) break;
        m_executedNodeIndexes.insert(arg);
    }
    return 0;
}

void CsExecutor::fatalError(const std::string& message) const
{
    Log(message);
    throw std::runtime_error{message};
}

int CsExecutor::RS_GetPersonParameterI(std::string_view person, std::string_view param) {
    if (person == "Hero") {
        auto it = m_heroStats.find(param);
        if (it != m_heroStats.cend()) {
            return it->second;
        }
    }
    return 0;
}

int CsExecutor::D_CloseDialog(int param) {
    return 0;
}

CsExecutor::ExecuteStatus CsExecutor::currentStatus() const {
    return m_currentStatus;
}

const char* CsExecutor::currentStatusString() const
{
    switch (m_currentStatus) {
        case kStart: return "Start";
        case kRestart: return "Restart";
        case kContinue: return "Continue";
        case kWaitUser: return "WaitUser";
        case kEnd: return "End";
        case kInfinity: return "Infinity";
    }
    return "Unknown";
}

bool CsExecutor::isNodeExecuted(int index) const {
    return m_executedNodeIndexes.contains(index);
}

float CsExecutor::executedPercent() const {
    return ((float)m_executedNodeIndexes.size() / (float)m_nodes.size()) * 100.0f;
}

UMapStringVar_t& CsExecutor::scriptVars() {
    return m_scriptVars;
}
