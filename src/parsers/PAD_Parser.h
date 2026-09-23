#pragma once
#include <string_view>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <array>

enum class PAD_AnimationTypeMask : uint32_t {
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

enum class PAD_AnimationDirection : uint32_t {
    up = 0,
    up_left,
    left,
    down_left,
    down,
    down_right,
    right,
    up_right
};

enum class PAD_AnimationDirectionGo : uint32_t {
    up = 0,
    up_up_left,       // 22.5°
    up_left,          // 45°
    left_up_left,     // 67.5°
    left,             // 90°
    left_down_left,   // 112.5°
    down_left,        // 135°
    down_down_left,   // 157.5°
    down,             // 180°
    down_down_right,  // 202.5°
    down_right,       // 225°
    right_down_right, // 247.5°
    right,            // 270°
    right_up_right,   // 292.5°
    up_right,         // 315°
    up_up_right       // 337.5°
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

static std::string_view animationDirectionToString(PAD_AnimationDirection type) {
    switch (type) {
        case PAD_AnimationDirection::up: return "up";
        case PAD_AnimationDirection::up_left: return "up_left";
        case PAD_AnimationDirection::left: return "left";
        case PAD_AnimationDirection::down_left: return "down_left";
        case PAD_AnimationDirection::down: return "down";
        case PAD_AnimationDirection::down_right: return "down_right";
        case PAD_AnimationDirection::right: return "right";
        case PAD_AnimationDirection::up_right: return "up_right";
    }
    return "unknown";
}

static std::string_view animationDirectionGoToString(PAD_AnimationDirectionGo type) {
    switch (type) {
        case PAD_AnimationDirectionGo::up: return animationDirectionToString(PAD_AnimationDirection::up);
        case PAD_AnimationDirectionGo::up_left: return animationDirectionToString(PAD_AnimationDirection::up_left);
        case PAD_AnimationDirectionGo::left: return animationDirectionToString(PAD_AnimationDirection::left);
        case PAD_AnimationDirectionGo::down_left: return animationDirectionToString(PAD_AnimationDirection::down_left);
        case PAD_AnimationDirectionGo::down: return animationDirectionToString(PAD_AnimationDirection::down);
        case PAD_AnimationDirectionGo::down_right: return animationDirectionToString(PAD_AnimationDirection::down_right);
        case PAD_AnimationDirectionGo::right: return animationDirectionToString(PAD_AnimationDirection::right);
        case PAD_AnimationDirectionGo::up_right: return animationDirectionToString(PAD_AnimationDirection::up_right);
        case PAD_AnimationDirectionGo::up_up_left: return "up_up_left";
        case PAD_AnimationDirectionGo::left_up_left: return "left_up_left";
        case PAD_AnimationDirectionGo::left_down_left: return "left_down_left";
        case PAD_AnimationDirectionGo::down_down_left: return "down_down_left";
        case PAD_AnimationDirectionGo::down_down_right: return "down_down_right";
        case PAD_AnimationDirectionGo::right_down_right: return "right_down_right";
        case PAD_AnimationDirectionGo::right_up_right: return "right_up_right";
        case PAD_AnimationDirectionGo::up_up_right: return "up_up_right";
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

    static bool isGoType(PAD_AnimationTypeMask type) {
        return (type == PAD_AnimationTypeMask::tb_go
                || type == PAD_AnimationTypeMask::rt_go);
    }

    static constexpr int kAnimationGoRowCount = 16;
    static constexpr int kAnimationRowCount = 8;

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

    static constexpr int kAnimationDirectionCount = 8;
    static constexpr std::array<PAD_AnimationDirection, kAnimationDirectionCount> animationDirectionsPerson = {
        PAD_AnimationDirection::up,
        PAD_AnimationDirection::up_left,
        PAD_AnimationDirection::left,
        PAD_AnimationDirection::down_left,
        PAD_AnimationDirection::down,
        PAD_AnimationDirection::down_left, // mirror
        PAD_AnimationDirection::left,      // mirror
        PAD_AnimationDirection::up_left    // mirror
    };

    static constexpr std::array<PAD_AnimationDirection, kAnimationDirectionCount> animationDirectionsShadow = {
        PAD_AnimationDirection::up,
        PAD_AnimationDirection::up_left,
        PAD_AnimationDirection::left,
        PAD_AnimationDirection::down_left,
        PAD_AnimationDirection::down,
        PAD_AnimationDirection::down_right,
        PAD_AnimationDirection::right,
        PAD_AnimationDirection::up_right
    };

    static constexpr int kAnimationDirectionGoCount = 16;
    static constexpr std::array<PAD_AnimationDirectionGo, kAnimationDirectionGoCount> animationDirectionsGoPerson = {
        PAD_AnimationDirectionGo::up,
        PAD_AnimationDirectionGo::up_up_left,
        PAD_AnimationDirectionGo::up_left,
        PAD_AnimationDirectionGo::left_up_left,
        PAD_AnimationDirectionGo::left,
        PAD_AnimationDirectionGo::left_down_left,
        PAD_AnimationDirectionGo::down_left,
        PAD_AnimationDirectionGo::down_down_left,
        PAD_AnimationDirectionGo::down,
        PAD_AnimationDirectionGo::down_down_left, // mirror
        PAD_AnimationDirectionGo::down_left,      // mirror
        PAD_AnimationDirectionGo::left_down_left, // mirror
        PAD_AnimationDirectionGo::left,           // mirror
        PAD_AnimationDirectionGo::left_up_left,   // mirror
        PAD_AnimationDirectionGo::up_left,        // mirror
        PAD_AnimationDirectionGo::up_up_left      // mirror
    };

    static constexpr std::array<PAD_AnimationDirectionGo, kAnimationDirectionGoCount> animationDirectionsGoShadow = {
        PAD_AnimationDirectionGo::up,
        PAD_AnimationDirectionGo::up_up_left,
        PAD_AnimationDirectionGo::up_left,
        PAD_AnimationDirectionGo::left_up_left,
        PAD_AnimationDirectionGo::left,
        PAD_AnimationDirectionGo::left_down_left,
        PAD_AnimationDirectionGo::down_left,
        PAD_AnimationDirectionGo::down_down_left,
        PAD_AnimationDirectionGo::down,
        PAD_AnimationDirectionGo::down_down_right,
        PAD_AnimationDirectionGo::down_right,
        PAD_AnimationDirectionGo::right_down_right,
        PAD_AnimationDirectionGo::right,
        PAD_AnimationDirectionGo::right_up_right,
        PAD_AnimationDirectionGo::up_right,
        PAD_AnimationDirectionGo::up_up_right
    };
};

class PAD_Parser {
public:
    PAD_Parser() = delete;

    static std::optional<PAD_Data> parse(std::string_view path, std::string* error);
};

