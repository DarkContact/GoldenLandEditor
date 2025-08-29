#include "StringUtils.h"

#include <algorithm>

std::string StringUtils::extractQuotedValue(const std::string& line) {
    size_t firstQuote = line.find('"');
    size_t lastQuote  = line.rfind('"');

    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }

    return {};
}

std::string StringUtils::toLower(std::string_view input) {
    std::string result(input);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}
