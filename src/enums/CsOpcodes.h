#pragma once
#include <cstdint>

enum OpcodeType {
    kUnknown = -1,
    kLogicOr = 0,
    kLogicXor = 1, // ??
    kLogicAnd = 2,
    kNumberLiteral = 24,
    kNumberVarName = 21,
    kStringLiteral = 22,
    kStringVarName = 23,
    kFunc = 48,
    kJmp = 49,
    kAssign = 50
};

enum OpcodeGroup {
    kLogical, // [0-2]
    kComparison, // [6-11]
    kArithmetic, // [14-18]
    kOther
};

const char* csOpcodeToString(int32_t opcode) noexcept;
OpcodeGroup csOpcodeToGroup(int32_t opcode) noexcept;
