#include "CsExecutor.h"

#include <cassert>
#include <format>

#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "windows/CsViewer.h"

CsExecutor::CsExecutor(std::span<const CS_Node> nodes) :
    m_nodes(nodes)
{
    assert(!m_nodes.empty());
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

// TODO: Нет смысла каждый раз читать этот файл. Вынести куда-то отдельно
bool CsExecutor::readGlobalVariables(std::string_view rootDirectory, std::string* error)
{
    std::string varsPath = std::format("{}/scripts/dialogs_special/zlato_vars.scr", rootDirectory);
    auto fileData = FileUtils::loadFile(varsPath, error);
    if (fileData.empty()) {
        return false;
    }

    std::string_view fileStringView((char*)fileData.data(), fileData.size());
    StringUtils::forEachLine(fileStringView, [this] (std::string_view line)
    {
        std::size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        line = StringUtils::trim(line);
        if (line.empty()) return;

        std::string_view type, name, value;
        if (!parse3(line, type, name, value)) {
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

        m_globalVars.emplace(std::string(name), varValue);
    });

    return true;
}

bool CsExecutor::readScriptVariables(std::string* error)
{
    for (const auto& node : m_nodes) {
        if (node.opcode == OpcodeType::kStringVarName) {
            if (auto it = m_globalVars.find(node.text); it != m_globalVars.end()) {
                m_scriptVars.emplace(node.text, it->second);
            }
        }
    }
    return true;
}

void CsExecutor::restart()
{
    m_counter = 0;
    m_currentNodeIndex = 0;
    m_funcs.clear();
    m_scriptVars.clear();
    readScriptVariables(nullptr);
}

int CsExecutor::currentNodeIndex() const
{
    return m_currentNodeIndex;
}

std::vector<std::string> CsExecutor::variablesInfo() const
{
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
    return m_funcs;
}

bool CsExecutor::next()
{
    // Защита от бесконечного выполнения
    if (m_counter >= CsExecutor::kStopCounter) {
        return false;
    }
    ++m_counter;

    const CS_Node& currentNode = m_nodes[m_currentNodeIndex];

    if (currentNode.opcode >= 6 && currentNode.opcode <= 11) { // Операторы сравнения
        const CS_Node& lNode = m_nodes[currentNode.a];
        Variable_t lValue;
        if (lNode.opcode == kStringVarName) {
            lValue = m_scriptVars[lNode.text];
        } else if (lNode.opcode == kNumberLiteral) {
            lValue = lNode.value;
        } else {
            assert(false);
        }

        const CS_Node& rNode = m_nodes[currentNode.b];
        Variable_t rValue;
        if (rNode.opcode == kStringVarName) {
            rValue = m_scriptVars[rNode.text];
        } else if (rNode.opcode == kNumberLiteral) {
            rValue = rNode.value;
        } else {
            assert(false);
        }

        bool isTrue = false;
        if (currentNode.opcode == 6) {
            isTrue = (lValue != rValue);
        } else if (currentNode.opcode == 7) {
            isTrue = (lValue == rValue);
        } else if (currentNode.opcode == 8) {
            isTrue = (lValue >= rValue);
        } else if (currentNode.opcode == 9) {
            isTrue = (lValue <= rValue);
        } else if (currentNode.opcode == 10) {
            isTrue = (lValue > rValue);
        } else if (currentNode.opcode == 11) {
            isTrue = (lValue < rValue);
        }

        m_currentNodeIndex = isTrue ? currentNode.d : currentNode.c;
    }

    if (currentNode.opcode == kJmp) {
        if (currentNode.d == -1)
            return false;

        m_currentNodeIndex = currentNode.c;
    } else if (currentNode.opcode == kAssign) {
        const CS_Node& rNode = m_nodes[currentNode.b];
        Variable_t rValue;
        if (rNode.opcode == kStringVarName) {
            rValue = m_scriptVars[rNode.text];
        } else if (rNode.opcode == kNumberLiteral) {
            rValue = (int)rNode.value; // TODO: Корректное приведение типов
        } else if (rNode.opcode == kFunc) {
            m_funcs.emplace_back(CsViewer::funcStr(rNode.value));
            rValue = 0; // Пока не реализовано, будет так
        } else {
            assert(false);
        }

        const CS_Node& lNode = m_nodes[currentNode.a];
        if (lNode.opcode == kStringVarName) {
            m_scriptVars[lNode.text] = rValue;
        } else {
            assert(false);
        }

        m_currentNodeIndex = currentNode.d;
    }
    return true;
}
