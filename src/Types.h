#pragma once
#include <cstdint>
#include <vector>
#include <string>

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
