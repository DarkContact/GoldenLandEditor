#pragma once
#include <string_view>
#include <optional>
#include <string>
#include <vector>

struct MDF_Params {
    int32_t p01 = 0; // Во всех файлах 00 00 00 01
    int32_t p02 = 0; // Во всех файлах 00 00 00 01, кроме shad_shadowwarrior (Там есть значение CC CC CC CC)
    int32_t flags = 0;
    int32_t nFrame = 0;
    int32_t p05 = 0;
    float delayMs = 0;
    float p07 = 0;
    float p08 = 0;
    float p09 = 0;
    int32_t animationTimeMs = 0;
};

struct MDF_Animation {
    int32_t framesCount = 0;
    int32_t xOffset = 0;
    int32_t yOffset = 0;
    int32_t a04 = 0;
    int32_t a05 = 0;
    int32_t startTimeMs = 0;
    int32_t endTimeMs = 0;
    std::string maskAnimationPath;
    std::string animationPath;
    std::vector<MDF_Params> params;
};

struct MDF_Layer {
    std::vector<MDF_Animation> animations;
};


struct MDF_Data {
    int32_t totalDurationMs = 0;
    std::vector<MDF_Layer> layers;
};

class MDF_Parser
{
public:
    MDF_Parser() = delete;

    static std::optional<MDF_Data> parse(std::string_view path, std::string* error);

    static constexpr int kMaxLayers = 5;
};
