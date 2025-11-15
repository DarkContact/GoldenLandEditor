#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct CS_Node {
    int32_t opcode = -1;
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
    int32_t d = 0;
    int32_t child[9] = {0};

    std::string text;
    double value;
};

struct CS_Data {
    std::vector<CS_Node> nodes;
};

class CS_Parser {
public:
    CS_Parser() = delete;

    static bool parse(std::string_view csPath, CS_Data& data, std::string* error);

    static const char* opcodeStr(int32_t opcode);
};

