#include "Resources.h"

#include <filesystem>
#include <format>

namespace fs = std::filesystem;

Resources::Resources(std::string_view rootDirectory) :
    m_rootDirectory(rootDirectory)
{}

std::vector<std::string> Resources::levelNames() const
{
    std::vector<std::string> results;

    try {
        for (const auto& entry : fs::directory_iterator(std::format("{}/levels/single", m_rootDirectory))) {
            results.push_back(entry.path().filename().string());
        }
    } catch (const fs::filesystem_error& e) {
        fprintf(stderr, "Filesystem error: %s\n", e.what());
    }

    return results;
}

std::vector<std::string> Resources::filesWithExtension(std::string_view extension) const
{
    std::vector<std::string> files;

    for (const auto& entry : fs::recursive_directory_iterator(m_rootDirectory)) {
        if (entry.path().extension() == extension) {
            files.push_back(entry.path().lexically_relative(m_rootDirectory).string());
        }
    }

    return files;
}
