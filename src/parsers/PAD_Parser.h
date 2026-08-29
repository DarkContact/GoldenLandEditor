#pragma once
#include <string_view>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

enum class PAD_AnimationTypeMask {
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

struct PAD_Data {
    uint32_t animationMask;
    std::vector<uint8_t> animationData;
    // TODO: Какие-то данные в хвосте
};

class PAD_Parser {
public:
    PAD_Parser() = delete;

    static std::optional<PAD_Data> parse(std::string_view path, std::string* error);
};

