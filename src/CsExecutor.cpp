#include "CsExecutor.h"

#include <format>

#include "utils/FileUtils.h"
#include "utils/StringUtils.h"

CsExecutor::CsExecutor(std::span<const CS_Node> nodes) :
    m_nodes(nodes)
{

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

        m_vars.emplace(std::string(name), varValue);
    });

    return true;
}
