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

            node.child[0] = readInt32(fileData, offset);
            if (node.child[0] != -1) {
                node.child[1] = readInt32(fileData, offset);
                if (node.child[1] != -1) {
                    node.child[2] = readInt32(fileData, offset);
                    if (node.child[2] != -1) {
                        node.child[3] = readInt32(fileData, offset);
                        if (node.child[3] != -1) {
                            node.child[4] = readInt32(fileData, offset);
                            if (node.child[4] != -1) {
                                node.child[5] = readInt32(fileData, offset);
                                if (node.child[5] != -1) {
                                    node.child[6] = readInt32(fileData, offset);
                                    if (node.child[6] != -1) {
                                        node.child[7] = readInt32(fileData, offset);
                                        if (node.child[7] != -1) {
                                            node.child[8] = readInt32(fileData, offset);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
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

const char* CS_Parser::opcodeStr(int32_t opcode)
{
    switch (opcode) {
        case 0: return "||";
        case 1: return "^^";
        case 2: return "&&";
        case 3: return "|";
        case 4: return "^";
        case 5: return "&";
        case 6: return "!=";
        case 7: return "==";
        case 8: return ">=";
        case 9: return "<=";
        case 10: return ">";
        case 11: return "<";
        case 12: return "<<";
        case 13: return ">>";
        case 14: return "+";
        case 15: return "-";
        case 16: return "*";
        case 17: return "/";
        case 18: return "%";
        case 19: return "~";
        case 20: return "!";
        case 21: return "realVar";
        case 22: return "str";
        case 23: return "strVar";
        case 24: return "real";
        case 48: return "func";
        case 49: return "49";
        case 50: return "50";
        default: return "unk";
    }
}
