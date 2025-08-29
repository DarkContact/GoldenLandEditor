#pragma once
#include <string>

class StringUtils
{
public:
    StringUtils() = delete;

    static std::string extractQuotedValue(const std::string& line);

    static std::string toLower(std::string_view input);
};

