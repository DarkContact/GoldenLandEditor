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
        StringUtils::formatToBuffer(additionInfo, "txt: \"{}\"", varLiteral);
    } else if (opcode == kStringVarName) {
        StringUtils::formatToBuffer(additionInfo, "txt: {}", text);
    } else if (opcode == kFunc) {
        constexpr int kArgsInfoSize = 128;
        char argsInfo[kArgsInfoSize];
        argsInfo[0] = '\0';
        size_t offset = 0;
        for (int j = 0; j < args.size(); j++) {
            int32_t idx = args[j];
            if (idx == -1) break;
            offset += StringUtils::formatToBuffer(std::span<char>(argsInfo + offset, kArgsInfoSize - offset), "{} ", idx);
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
        if (node.opcode >= 0 && node.opcode <= 20 || node.opcode == kAssign) {
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
        }

        data.nodes.push_back(std::move(node));
    }
    assert(offset == fileSize);

    return true;
}

bool CS_Parser::save(std::string_view csPath, const CS_Data& data, std::string* error)
{
    Tracy_ZoneScoped;

    std::vector<uint8_t> saveData;
    writeUInt32(saveData, 0); // Пока не знаем размер, запишем в конце
    for (const auto& node : data.nodes) {
        writeInt32(saveData, node.opcode);
        if (node.opcode >= 0 && node.opcode <= 20 || node.opcode == kAssign) {
            writeInt32(saveData, node.a);
            writeInt32(saveData, node.b);
            writeInt32(saveData, node.c);
            writeInt32(saveData, node.d);
        } else if (node.opcode == kNumberVarName || node.opcode == kNumberLiteral) {
            writeDouble(saveData, node.value);
        } else if (node.opcode == kStringVarName || node.opcode == kStringLiteral) {
            writeCString(saveData, node.text);
        } else if (node.opcode == kFunc) {
            writeInt32(saveData, node.c);
            writeInt32(saveData, node.d);
            writeDouble(saveData, node.value);

            for (auto arg : node.args) {
                writeInt32(saveData, arg);
                if (arg == -1)
                    break;
            }
        } else if (node.opcode == kJmp) {
            writeInt32(saveData, node.c);
            writeInt32(saveData, node.d);
        }
    }

    // Пишем размер
    uint32_t size = static_cast<uint32_t>(saveData.size());
    std::memcpy(saveData.data(), &size, sizeof(uint32_t));

    return FileUtils::saveFile(csPath, saveData, error);
}
