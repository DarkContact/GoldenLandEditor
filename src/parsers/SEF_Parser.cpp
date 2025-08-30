#include "SEF_Parser.h"

#include <fstream>
#include <iostream>

#include "StringUtils.h"

SEF_Parser::SEF_Parser(std::string_view sefPath) {
    std::ifstream file(sefPath.data());
    if (!file) {
        fprintf(stderr, "Error: cannot open file: %s\n", sefPath);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("pack:")) {
            m_data.pack = StringUtils::extractQuotedValue(line);
        }
    }
}

SEF_Data SEF_Parser::data() const
{
    return m_data;
}
