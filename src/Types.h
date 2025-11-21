#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <format>

#include "SDL3/SDL_timer.h"
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

template <>
struct std::formatter<std::string> : std::formatter<std::string_view> {
    auto format(const std::string& s, format_context& ctx) const {
        return formatter<std::string_view>::format(s, ctx);
    }
};

template <typename T>
struct std::formatter<std::vector<T>> : std::formatter<T> {
    auto format(const std::vector<T>& vec, auto& ctx) const {
        auto out = ctx.out();
        *out++ = '[';

        for (size_t i = 0; i < vec.size(); ++i) {
            out = std::formatter<T>::format(vec[i], ctx);
            if (i + 1 != vec.size()) {
                *out++ = ','; *out++ = ' ';
            }
        }
        *out++ = ']';
        return out;
    }
};

