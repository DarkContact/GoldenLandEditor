#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <array>

class Resources
{
public:
    Resources(std::string_view rootDirectory);

    std::vector<std::string> levelNames() const;

    std::vector<std::string> sdbFiles() const;
    std::vector<std::string> csxFiles() const;
    std::vector<std::string> filesWithExtension(std::string_view extension) const;

private:
    std::string_view m_rootDirectory;
    std::array<std::string, 14> m_mainDirectories;
};

