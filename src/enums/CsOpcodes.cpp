#include "CsOpcodes.h"

const char* csOpcodeToString(int32_t opcode) noexcept
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
        case 21: return "num_var";
        case 22: return "str";
        case 23: return "str_var";
        case 24: return "num";
        case 48: return "func";
        case 49: return "jmp";
        case 50: return "assign";
        default: return "unknown";
    }
}

OpcodeGroup csOpcodeToGroup(int32_t opcode) noexcept
{
    if (opcode >= 0 && opcode <= 2) {
        return kLogical;
    } else if (opcode >= 6 && opcode <= 11) {
        return kComparison;
    } else if (opcode >= 14 && opcode <= 18) {
        return kArithmetic;
    }
    return kOther;
}
