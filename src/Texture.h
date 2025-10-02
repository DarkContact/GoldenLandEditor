#pragma once
#include <string>

#include "SDL3/SDL_render.h"

class Texture {
public:
    Texture() noexcept = default;
    ~Texture() noexcept;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;


    static Texture create(SDL_Renderer* renderer,
                          SDL_PixelFormat format,
                          SDL_TextureAccess access,
                          int width,
                          int height,
                          std::string* error = nullptr) noexcept;

    static Texture createFromSurface(SDL_Renderer *renderer,
                                     SDL_Surface *surface,
                                     std::string* error = nullptr) noexcept;

    bool updatePixels(const void* pixels, const SDL_Rect* rect = nullptr, std::string* error = nullptr) noexcept;

    bool isValid() const noexcept { return m_texture; }
    explicit operator bool() const noexcept { return m_texture; }

    SDL_Texture* get() const noexcept { return m_texture; }

    const SDL_Texture* operator->() const { return m_texture; }

private:
    void freeTexture();

    SDL_Texture* m_texture = nullptr;
};

