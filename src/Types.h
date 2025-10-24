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
    uint32_t delay = 0;

    const Texture& currentTexture() const {
        return textures[currentFrame];
    }

    void update(uint64_t timeMs = SDL_GetTicks()) {
        if (lastUpdate == 0) {
            lastUpdate = timeMs;
            return;
        }

        if (timeMs - lastUpdate >= delay) {
            nextFrame();
            lastUpdate = timeMs;
        }
    }

    void stop() {
        currentFrame = 0;
        lastUpdate = 0;
    }

private:
    void nextFrame() {
        ++currentFrame;
        if (currentFrame == textures.size()) {
            currentFrame = 0;
        }
    }

    uint32_t currentFrame = 0;
    uint64_t lastUpdate = 0;
};
