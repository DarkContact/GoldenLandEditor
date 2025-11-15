#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct CS_Node {
    int32_t opcode;
    int32_t a;
    int32_t b;
    int32_t c;
    int32_t d;
    CS_Node* child[9];
    std::string text;
};

struct CS_Data {
    std::vector<CS_Node> nodes;
};

class CS_Parser {
public:
    CS_Parser() = delete;

    static bool parse(std::string_view csPath, CS_Data& data, std::string* error);
};

