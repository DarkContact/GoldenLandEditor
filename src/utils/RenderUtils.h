#pragma once
#include <cstdint>

struct SDL_Renderer;

namespace RenderUtils {
    uint64_t getUsedVideoMemoryBytes(SDL_Renderer* renderer);
}
