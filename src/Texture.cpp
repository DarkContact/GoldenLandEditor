#include "Texture.h"

#include <format>

#include "SDL3/SDL_assert.h"

Texture::~Texture() noexcept
{
    freeTexture();
}

Texture::Texture(Texture&& other) noexcept
{
    freeTexture();
    m_texture = other.m_texture;
    other.m_texture = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    freeTexture();
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
        *error = std::format("{}", SDL_GetError());
    }
    return texture;
}

Texture Texture::createFromSurface(SDL_Renderer* renderer, SDL_Surface* surface, std::string* error) noexcept
{
    Texture texture;
    texture.m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture.m_texture && error) {
        *error = std::format("{}", SDL_GetError());
    }
    return texture;
}

bool Texture::updatePixels(const void* pixels, const SDL_Rect* rect, std::string* error) noexcept
{
    SDL_assert(isValid());

    SDL_PropertiesID props = SDL_GetTextureProperties(m_texture);
    SDL_TextureAccess access = static_cast<SDL_TextureAccess>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, 0));
    SDL_PixelFormat format = static_cast<SDL_PixelFormat>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, 0));

    SDL_assert(access == SDL_TEXTUREACCESS_STATIC);
    int bytesPerPixel = SDL_BYTESPERPIXEL(format);

    int updatedWidth = rect ? rect->w : m_texture->w;
    bool isOk = SDL_UpdateTexture(m_texture, rect, pixels, bytesPerPixel * updatedWidth);
    if (!isOk && error) {
        *error = std::format("{}", SDL_GetError());
    }
    return isOk;
}

void Texture::freeTexture() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}
