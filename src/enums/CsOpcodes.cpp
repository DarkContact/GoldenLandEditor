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

bool csOpcodeIsValid(int32_t opcode) noexcept {
    switch (opcode) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 48:
        case 49:
        case 50:
            return true;
    }
    return false;
}
