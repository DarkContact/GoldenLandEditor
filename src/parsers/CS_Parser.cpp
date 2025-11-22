#include "CS_Parser.h"

#include <cassert>

#include "enums/CsOpcodes.h"

#include "utils/IoUtils.h"
#include "utils/FileUtils.h"
#include "utils/TracyProfiler.h"

using namespace IoUtils;

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
