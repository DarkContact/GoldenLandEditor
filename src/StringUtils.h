#pragma once
#include <string>

class StringUtils
{
public:
    StringUtils() = delete;

    static std::string extractQuotedValue(const std::string& line);
};

