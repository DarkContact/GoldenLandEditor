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
    std::vector<std::string> mdfFiles() const;

private:
    std::vector<std::string> filesWithExtension(std::initializer_list<int> indices, std::string_view extension) const;
    std::vector<std::string> filesWithExtensionAsync(std::initializer_list<int> indices, std::string_view extension) const;

    std::string_view m_rootDirectory;
    std::array<std::string, 14> m_mainDirectories;
};

