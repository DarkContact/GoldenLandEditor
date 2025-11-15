#include "CS_Parser.h"

#include <cassert>

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
        } else if (node.opcode == 21 || node.opcode == 24) {
            node.value = readDouble(fileData, offset);
        } else if (node.opcode == 22 || node.opcode == 23) {
            std::string_view text = readCString(fileData, offset);
            node.text = std::string(text);
        } else if (node.opcode == 48) {
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
            node.value = readDouble(fileData, offset);

            for (int i = 0; i < node.child.size(); ++i) {
                node.child[i] = readInt32(fileData, offset);
                if (node.child[i] == -1)
                    break;
            }
        } else if (node.opcode == 49) {
            node.c = readInt32(fileData, offset);
            node.d = readInt32(fileData, offset);
        } else if (node.opcode == 50) {
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
