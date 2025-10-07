#pragma once
#include <string_view>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

struct AnimationInfo {
    uint32_t height = 0;
    uint32_t duration = 0;
};

struct LAO_Data {
    std::vector<AnimationInfo> infos;
};

class LAO_Parser
{
public:
    LAO_Parser() = delete;

    static std::optional<LAO_Data> parse(std::string_view laoPath, std::string* error);
};

