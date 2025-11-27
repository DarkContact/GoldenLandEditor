#include "CS_Parser.h"

#include <format>
#include <cassert>

#include "enums/CsFunctions.h"
#include "enums/CsOpcodes.h"

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "utils/TracyProfiler.h"

using namespace IoUtils;

void CS_Node::toStringBuffer(std::span<char> buffer, bool showDialogPhrases, const std::map<int, std::string>& sdbDialogStrings) const {
    char additionInfo[3584];
    if (opcode >= 0 && opcode <= 20) {
        StringUtils::formatToBuffer(additionInfo, "a: {}, b: {}, c: {}, d: {}", a, b, c, d);
    } else if (opcode == kNumberLiteral || opcode == kNumberVarName) {
        if (showDialogPhrases && !sdbDialogStrings.empty() && opcode == kNumberLiteral) {
            auto it = sdbDialogStrings.find(value);
            if (it != sdbDialogStrings.cend()) {
                StringUtils::formatToBuffer(additionInfo, "val: {} [{}]", value, it->second);
            } else {
                StringUtils::formatToBuffer(additionInfo, "val: {}", value);
            }
        } else {
            StringUtils::formatToBuffer(additionInfo, "val: {}", value);
        }
    } else if (opcode == kStringLiteral) {
        char varLiteral[320];
        StringUtils::decodeWin1251ToUtf8Buffer(text, varLiteral);
        StringUtils::formatToBuffer(additionInfo, "txt: {}", varLiteral);
    } else if (opcode == kStringVarName) {
        StringUtils::formatToBuffer(additionInfo, "txt: {}", text);
    } else if (opcode == kFunc) {
        char argsInfo[128];
        argsInfo[0] = '\0';
        for (int j = 0; j < args.size(); j++) {
            int32_t idx = args[j];
            if (idx == -1) break;
            StringUtils::formatToBuffer(argsInfo, "{} ", idx);
        }
        std::string_view funcStr = csFuncToString(value);
        StringUtils::formatToBuffer(additionInfo, "val: [{}], c: {}, d: {}, args: [{}]", funcStr, c, d, StringUtils::trimRight(argsInfo));
    } else if (opcode == kJmp) {
        StringUtils::formatToBuffer(additionInfo, "c: {}, d: {}", c, d);
    } else if (opcode == kAssign) {
        StringUtils::formatToBuffer(additionInfo, "a: {}, b: {}, c: {}, d: {}", a, b, c, d);
    }

    StringUtils::formatToBuffer(buffer, "Opcode: {} [{}] | {}", opcode, csOpcodeToString(opcode), additionInfo);
}

bool CS_Parser::parse(std::string_view csPath, CS_Data& data, std::string* error)
{
    Tracy_ZoneScoped;
    auto fileData = FileUtils::loadFile(csPath, error);
    if (fileData.empty()) {
        return false;
    }

    size_t offset = 0;
    size_t fileSize = readUInt32(fileData, offset);
    assert(fileData.size() == fileSize);

    while(offset < fileSize) {
        CS_Node node;

        node.opcode = readInt32(fileData, offset);
        if (node.opcode >= 0 && node.opcode <= 20) {
            node.a = readInt32(fileData, offset);
            node.b = readInt32(fileData, offset);
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
        } else if (node.opcode == kNumberVarName || node.opcode == kNumberLiteral) {
            node.value = readDouble(fileData, offset);
        } else if (node.opcode == kStringVarName || node.opcode == kStringLiteral) {
            std::string_view text = readCString(fileData, offset);
            node.text = std::string(text);
        } else if (node.opcode == kFunc) {
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
            node.value = readDouble(fileData, offset);

            for (int i = 0; i < node.args.size(); ++i) {
                node.args[i] = readInt32(fileData, offset);
                if (node.args[i] == -1)
                    break;
            }
        } else if (node.opcode == kJmp) {
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
        } else if (node.opcode == kAssign) {
            node.a = readInt32(fileData, offset);
            node.b = readInt32(fileData, offset);
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
        }

        data.nodes.push_back(std::move(node));
    }
    assert(offset == fileSize);

    return true;
}
