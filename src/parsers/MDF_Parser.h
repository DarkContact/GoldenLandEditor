#pragma once
#include <string_view>
#include <optional>
#include <string>
#include <vector>

struct MDF_Params {
    int32_t p01 = 0;
    int32_t p02 = 0;
    int32_t p03 = 0;
    int32_t p04 = 0;
    int32_t p05 = 0;
    float p06 = 0;
    int32_t p07 = 0;
    int32_t p08 = 0;
    int32_t p09 = 0;
    int32_t p10 = 0;
};

struct MDF_Pack {
    int32_t a01 = 0;
    int32_t a02 = 0;
    int32_t a03 = 0;
    int32_t a04 = 0;
    int32_t a05 = 0;
    int32_t a06 = 0;
    int32_t a07 = 0;
    std::string maskAnimationPath;
    std::string animationPath;
    std::vector<MDF_Params> params;
};

struct MDF_Entry {
    std::vector<MDF_Pack> packs;
};


struct MDF_Data {
    int32_t height = 0;
    std::vector<MDF_Entry> entries;
};

class MDF_Parser
{
public:
    MDF_Parser() = delete;

    static std::optional<MDF_Data> parse(std::string_view path, std::string* error);

    static constexpr int kMaxEntries = 5;
};
