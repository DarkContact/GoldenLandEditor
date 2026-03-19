#pragma once
#include <unordered_map>
#include <string>

enum Instruction {
    kExec,
    kUserInputAndExec,
    kSetVariable,
    kSoftRestart,
    kHardRestart
};

struct DialogInstruction {
    Instruction inst;
    int value = 0;
    std::string text;
};

std::unordered_map<std::string, std::vector<DialogInstruction>> getManualDialogTestData();

