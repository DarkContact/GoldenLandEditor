#pragma once
#include <string_view>
#include <string>
#include <vector>

class Resources
{
public:
    Resources(std::string_view rootDirectory);

    std::vector<std::string> levelNames() const;

    std::vector<std::string> filesWithExtension(std::string_view extension) const;

private:
    std::string_view m_rootDirectory;
};

