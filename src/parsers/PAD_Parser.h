#pragma once
#include <string_view>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <array>

enum PAD_AnimationTypeMask : uint32_t {
    rt_stay =   0x00000001,
    rt_fun =    0x00000002,
    tb_stay =   0x00000004,
    tb_fun =    0x00000008,
    tb_go =     0x00000010,
    rt_go =     0x00000020,
    cast =      0x00000040,
    suffer =    0x00000080,
    die =       0x00000100,
    ss_attack = 0x00000400,

    hits0 =     0x00010000,
    hits1 =     0x00020000,
    hits2 =     0x00040000,
    hits3 =     0x00080000,
};

static std::string_view animationTypeMaskToString(PAD_AnimationTypeMask type) {
    switch (type) {
        case PAD_AnimationTypeMask::rt_stay: return "rt_stay";
        case PAD_AnimationTypeMask::rt_fun: return "rt_fun";
        case PAD_AnimationTypeMask::tb_stay: return "tb_stay";
        case PAD_AnimationTypeMask::tb_fun: return "tb_fun";
        case PAD_AnimationTypeMask::tb_go: return "tb_go";
        case PAD_AnimationTypeMask::rt_go: return "rt_go";
        case PAD_AnimationTypeMask::cast: return "cast";
        case PAD_AnimationTypeMask::suffer: return "suffer";
        case PAD_AnimationTypeMask::die: return "die";
        case PAD_AnimationTypeMask::ss_attack: return "ss_attack";
        case PAD_AnimationTypeMask::hits0: return "hits0";
        case PAD_AnimationTypeMask::hits1: return "hits1";
        case PAD_AnimationTypeMask::hits2: return "hits2";
        case PAD_AnimationTypeMask::hits3: return "hits3";
    }
    return "unknown";
}

struct PAD_CropsFrame {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

struct PAD_ShadowFrame {
    int32_t width;
    int32_t height;
    int32_t anchorX;
    int32_t anchorY;
};

struct PAD_Animation {
    PAD_AnimationTypeMask type;
    int32_t delay;
    int32_t framesPerRow;
    int32_t frameWidth;
    int32_t frameHeight;
    int32_t anchorX;
    int32_t anchorY;
    float movementX;
    float movementY;
    int32_t rowCount;
    std::vector<std::vector<PAD_CropsFrame>> crops;
    PAD_ShadowFrame shadowFrame;
    int32_t shadowRowCount;
    std::vector<std::vector<PAD_CropsFrame>> shadowCrops;
};

struct PAD_Data {
    std::vector<PAD_Animation> animations;

    static constexpr int kTypeMasksCount = 14;
    static constexpr std::array<PAD_AnimationTypeMask, kTypeMasksCount> typeMasks = {
        PAD_AnimationTypeMask::rt_stay,
        PAD_AnimationTypeMask::rt_fun,
        PAD_AnimationTypeMask::tb_stay,
        PAD_AnimationTypeMask::tb_fun,
        PAD_AnimationTypeMask::tb_go,
        PAD_AnimationTypeMask::rt_go,
        PAD_AnimationTypeMask::cast,
        PAD_AnimationTypeMask::suffer,
        PAD_AnimationTypeMask::die,
        PAD_AnimationTypeMask::ss_attack,
        PAD_AnimationTypeMask::hits0,
        PAD_AnimationTypeMask::hits1,
        PAD_AnimationTypeMask::hits2,
        PAD_AnimationTypeMask::hits3
    };
};

class PAD_Parser {
public:
    PAD_Parser() = delete;

    static std::optional<PAD_Data> parse(std::string_view path, std::string* error);
};

