#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <map>

struct CS_Node {
    int32_t opcode = -1;
    int32_t a = -1;
    int32_t b = -1;
    int32_t c = -1;
    int32_t d = -1;
    std::array<int32_t, 9> args = {-1, -1, -1,
                                   -1, -1, -1,
                                   -1, -1, -1};
    std::string text;
    double value = -1.0;

    std::string toString(bool showDialogPhrases, const std::map<int, std::string>& sdbDialogStrings = {}) const;
};

struct CS_Data {
    std::vector<CS_Node> nodes;
};

class CS_Parser {
public:
    CS_Parser() = delete;

    static bool parse(std::string_view csPath, CS_Data& data, std::string* error);
};

