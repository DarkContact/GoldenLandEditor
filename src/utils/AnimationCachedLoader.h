#pragma once
#include <unordered_map>
#include <string_view>
#include <vector>
#include <span>

#include "Texture.h"

struct SDL_Renderer;

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

