#pragma once
#include <string_view>
#include <vector>
#include <span>

#include "Texture.h"
#include "Types.h"

struct SDL_Renderer;

class AnimationCachedLoader
{
public:
    AnimationCachedLoader() = default;
    ~AnimationCachedLoader() = default;

    std::span<const Texture> loadAnimationCount(std::string_view filepath,
                                                int count,
                                                SDL_Renderer* renderer,
                                                const SDL_Color* transparentColor,
                                                std::string* error);

    void clear();

private:
    StringHashTable<std::vector<Texture>> m_cache;
};

