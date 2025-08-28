#pragma once
#include <string>

struct SEF_Data {
    std::string pack;
};

class SEF_Parser
{
public:
    SEF_Parser(std::string_view sefPath);

    SEF_Data data() const;

private:
    SEF_Data m_data;
};
