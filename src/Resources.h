#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <array>
#include <map>

#include "Types.h"

class Resources
{
public:
    Resources(std::string_view rootDirectory);

    std::vector<std::string> levelNames(LevelType type) const;
    std::vector<std::string> sdbFiles() const;
    std::vector<std::string> csxFiles() const;
    std::vector<std::string> mdfFiles() const;
    std::vector<std::string> csFiles() const;

    StringHashTable<std::string> levelHumanNameDictionary() const;
    std::map<int, std::string> dialogPhrases() const;
    StringHashTable<AgeVariable_t> globalVars() const;

private:
    std::vector<std::string> filesWithExtension(std::initializer_list<int> indices, std::string_view extension) const;
    std::vector<std::string> filesWithExtensionAsync(std::initializer_list<int> indices, std::string_view extension) const;

    std::string_view m_rootDirectory;
    std::array<std::string, 15> m_mainDirectories;
};

