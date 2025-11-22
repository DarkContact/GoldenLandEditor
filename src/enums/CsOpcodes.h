#pragma once
#include <cstdint>

enum OpcodeType {
    kUnknown = -1,
    // [0 - 20] Операторы
    kNumberLiteral = 24,
    kNumberVarName = 21,
    kStringLiteral = 22,
    kStringVarName = 23,
    kFunc = 48,
    kJmp = 49,
    kAssign = 50
};

const char* csOpcodeToString(int32_t opcode) noexcept;
