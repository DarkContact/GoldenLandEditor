#pragma once
#include <unordered_map>
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
    std::unordered_map<std::string, std::vector<Texture>, StringViewHash, StringViewEqual> m_cache;
};

