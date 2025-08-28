#include "Resources.h"

#include <filesystem>
#include <iostream>

Resources::Resources(std::string_view rootDirectory) :
    m_rootDirectory(rootDirectory)
{}

std::vector<std::string> Resources::levelNames() const
{
    std::vector<std::string> results;

    try {
        std::string path = std::string(m_rootDirectory) + std::string("/levels/single");
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                results.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }

    return results;
}
