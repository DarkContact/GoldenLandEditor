#include "Texture.h"

#include <format>

Texture::~Texture() noexcept
{
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}

Texture::Texture(Texture&& other) noexcept
{
    m_texture = other.m_texture;
    other.m_texture = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    m_texture = other.m_texture;
    other.m_texture = nullptr;
    return *this;
}

Texture Texture::create(
    SDL_Renderer* renderer,
    SDL_PixelFormat format,
    SDL_TextureAccess access,
    int width,
    int height,
    std::string* error) noexcept
{
    Texture texture;
    texture.m_texture = SDL_CreateTexture(renderer, format, access, width, height);
    if (!texture.m_texture && error) {
        *error = std::format("Failed to create texture: {}", SDL_GetError());
    }
    return texture;
}

Texture Texture::createFromSurface(SDL_Renderer* renderer, SDL_Surface* surface, std::string* error) noexcept
{
    Texture texture;
    texture.m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture.m_texture && error) {
        *error = std::format("Failed to create texture from surface: {}", SDL_GetError());
    }
    return texture;
}
