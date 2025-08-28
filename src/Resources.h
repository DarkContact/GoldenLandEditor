#pragma once
#include <string_view>
#include <string>
#include <vector>

class Resources
{
public:
    Resources(std::string_view rootDirectory);

    std::vector<std::string> levelNames() const;

    std::string levelSef(std::string_view level, std::string_view levelType = "single") const;

private:
    std::string_view m_rootDirectory;
};

