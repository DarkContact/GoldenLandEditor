#pragma once
#include <unordered_map>
#include <cstdint>
#include <variant>
#include <vector>
#include <string>

struct TilePosition {
    uint16_t x;
    uint16_t y;
};

struct CellGroup {
    std::string name;
    std::vector<TilePosition> cells;
};

enum class LevelType {
    kSingle,
    kMultiplayer
};

static std::string_view levelTypeToString(LevelType levelType) {
    static constexpr auto kLevelTypeSingle = std::string_view("single");
    static constexpr auto kLevelTypeMulti = std::string_view("multiplayer");

    if (levelType == LevelType::kSingle) {
        return kLevelTypeSingle;
    } else {
        return kLevelTypeMulti;
    }
}

struct StringViewHash {
    using is_transparent = std::true_type;

    std::size_t operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
};

struct StringViewEqual {
    using is_transparent = std::true_type;

    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
        return lhs == rhs;
    }
};

template <typename Value>
using StringHashTable = std::unordered_map<std::string, Value,
                        StringViewHash, StringViewEqual>;

using AgeVariable_t = std::variant<int32_t, uint32_t, double, std::string>;
