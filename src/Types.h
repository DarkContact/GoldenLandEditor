#pragma once
#include <unordered_map>
#include <cstdint>
#include <variant>
#include <vector>
#include <string>
#include <format>
#include <ranges>

#include <SDL3/SDL_timer.h>

#include "utils/StringUtils.h"
#include "Texture.h"

struct TilePosition {
    uint16_t x;
    uint16_t y;
};

struct CellGroup {
    std::string name;
    std::vector<TilePosition> cells;
};

struct BaseAnimation {
    std::vector<Texture> textures;
    uint32_t delayMs = 0;

    const Texture& currentTexture() const {
        return textures[currentFrame];
    }

    void update(uint64_t timeMs = SDL_GetTicks()) {
        if (lastUpdateTimeMs == 0) {
            lastUpdateTimeMs = timeMs;
            return;
        }

        if (timeMs - lastUpdateTimeMs >= delayMs) {
            nextFrame();
            lastUpdateTimeMs = timeMs;
        }
    }

    void stop() {
        currentFrame = 0;
        lastUpdateTimeMs = 0;
    }

protected:
    void nextFrame() {
        ++currentFrame;
        if (currentFrame == textures.size()) {
            currentFrame = 0;
        }
    }

    uint32_t currentFrame = 0;
    uint64_t lastUpdateTimeMs = 0;
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

template <typename R>
concept FormattableRange =
    std::ranges::range<R> &&
    !std::is_convertible_v<R, std::wstring_view> &&
    !std::is_convertible_v<R, std::string_view>;

template <FormattableRange R>
struct std::formatter<R> {
public:
    constexpr auto parse(auto& ctx) {
        return element_formatter.parse(ctx);
    }

    auto format(const R& range, auto& ctx) const {
        auto out = ctx.out();
        *out++ = '[';

        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        if (it != end) {
            out = element_formatter.format(*it, ctx);
            ++it;

            while (it != end) {
                *out++ = ',';
                *out++ = ' ';

                out = element_formatter.format(*it, ctx);
                ++it;
            }
        }

        *out++ = ']';
        return out;
    }

private:
    using T = std::ranges::range_value_t<R>;
    std::formatter<T> element_formatter;
};

template <>
struct std::formatter<AgeVariable_t> : std::formatter<std::string_view> {
    auto format(const AgeVariable_t& v, std::format_context& ctx) const {
        char buffer[1024];

        std::visit([&buffer](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int32_t>)
                StringUtils::formatToBuffer(buffer, "int32: {}", arg);
            else if constexpr (std::is_same_v<T, uint32_t>)
                StringUtils::formatToBuffer(buffer, "uint32: {}", arg);
            else if constexpr (std::is_same_v<T, double>)
                StringUtils::formatToBuffer(buffer, "double: {}", arg);
            else if constexpr (std::is_same_v<T, std::string>)
                StringUtils::formatToBuffer(buffer, "string: \"{}\"", arg);
        }, v);

        return std::formatter<std::string_view>::format(std::string_view{buffer}, ctx);
    }
};
