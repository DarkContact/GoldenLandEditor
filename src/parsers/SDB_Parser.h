#pragma once
#include <string>
#include <map>

struct SDB_Data {
    std::map<int, std::string> strings;
};

class SDB_Parser
{
public:
    SDB_Parser() = delete;

    static bool parse(std::string_view sdbPath, SDB_Data& data, std::string* error = nullptr);
};
