#include "AnimationCachedLoader.h"

#include "TextureLoader.h"

std::span<const Texture> AnimationCachedLoader::loadAnimationCount(std::string_view filepath,
                                                                   int count,
                                                                   SDL_Renderer* renderer,
                                                                   const SDL_Color* transparentColor,
                                                                   std::string* error)
{
    if (auto it = m_cache.find(filepath); it != m_cache.end()) {
        return it->second;
    }

    std::vector<Texture> textures;
    bool isOk = false;
    if (filepath.ends_with(".csx")) {
        isOk = TextureLoader::loadCountAnimationFromCsxFile(filepath, count, renderer, textures, error);
    } else if (filepath.ends_with(".bmp")) {
        isOk = TextureLoader::loadCountAnimationFromBmpFile(filepath, count, renderer, textures, transparentColor, error);
    } else {
        if (error) { *error = "Format unsupported!"; }
    }

    if (isOk) {
        auto [insertIt, _] = m_cache.emplace(std::string(filepath), std::move(textures));
        return insertIt->second;
    }

    return {};
}

void AnimationCachedLoader::clear()
{
    m_cache.clear();
}
