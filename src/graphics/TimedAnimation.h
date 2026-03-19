#pragma once
#include <cassert>
#include <span>

#include "Texture.h"

struct TimedAnimation {

    int width() const { return this->textures.front()->w; }
    int height() const { return this->textures.front()->h; }

    void setTextures(std::span<const Texture> textures) {
        this->textures = textures;
        assert(!this->textures.empty());
    }

    const Texture& currentTexture() const {
        assert(currentTimeMs >= 0 && currentTimeMs < totalDurationMs);
        int correctedTime = currentTimeMs - startTimeMs;
        int currentFrame = correctedTime / delayMs;

        if (currentFrame >= textures.size()) {
            currentFrame = textures.size() - 1;
        }

        // dark_boiling_blood
        // dark_sendlife
        // elem_oledinenie
        if (isReverse) {
            currentFrame = textures.size() - 1 - currentFrame;

            if (currentFrame < 0) {
                currentFrame = 0;
            }
        }

        return textures[currentFrame];
    }

    bool isActive() const { return active; }

    void update(uint64_t currentTimeMs) {
        assert(totalDurationMs > 0);

        this->currentTimeMs = currentTimeMs;
        active = (currentTimeMs >= startTimeMs && currentTimeMs < endTimeMs);
    }

    void setTimes(float delayMs, uint64_t startTimeMs, uint64_t endTimeMs, uint64_t totalDurationMs, bool isReverse) {
        this->delayMs = delayMs;
        this->startTimeMs = startTimeMs;
        this->endTimeMs = endTimeMs;
        this->totalDurationMs = totalDurationMs;
        this->isReverse = isReverse;
    }

    uint64_t getTotalDurationMs() const {
        return totalDurationMs;
    }

private:
    std::span<const Texture> textures;
    float delayMs = 0;
    uint64_t startTimeMs = 0;
    uint64_t endTimeMs = 0;
    uint64_t totalDurationMs = 0;
    bool isReverse = false;

    uint64_t currentTimeMs = 0;
    bool active = false;
};

