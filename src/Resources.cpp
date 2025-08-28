#include "Resources.h"

#include <filesystem>
#include <format>

Resources::Resources(std::string_view rootDirectory) :
    m_rootDirectory(rootDirectory)
{}

std::vector<std::string> Resources::levelNames() const
{
    std::vector<std::string> results;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(std::format("{}/levels/single", m_rootDirectory))) {
            if (entry.is_directory()) {
                results.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        fprintf(stderr, "Filesystem error: %s", e.what());
    }

    return results;
}


