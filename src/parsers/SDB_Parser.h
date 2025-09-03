#pragma once
#include <string>
#include <map>

struct SDB_Data {
    std::map<int, std::string> strings;
};

class SDB_Parser
{
public:
    SDB_Parser(std::string_view sdbPath);

    SDB_Data& parse();

private:
    std::string m_filePath;
    SDB_Data m_data;
};
