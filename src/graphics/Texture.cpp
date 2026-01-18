#include "Texture.h"

#include <cassert>
#include <cstring>

#include "utils/TracyProfiler.h"

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

Texture Texture::create(SDL_Renderer* renderer, SDL_PixelFormat format, SDL_TextureAccess access, int width, int height, std::string* error) noexcept
{
    Tracy_ZoneScoped;
    Texture texture;
    texture.m_texture = SDL_CreateTexture(renderer, format, access, width, height);
    if (!texture.isValid() && error) {
        *error = SDL_GetError();
    }
    return texture;
}

Texture Texture::createFromSurface(SDL_Renderer* renderer, SDL_Surface* surface, std::string* error) noexcept
{
    Tracy_ZoneScoped;
    Texture texture;
    texture.m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture.isValid() && error) {
        *error = SDL_GetError();
    }
    return texture;
}

bool Texture::updatePixels(const void* pixels, const SDL_Rect* rect, std::string* error) noexcept
{
    Tracy_ZoneScoped;
    assert(isValid());

    SDL_PropertiesID props = SDL_GetTextureProperties(m_texture);
    SDL_TextureAccess access = static_cast<SDL_TextureAccess>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, 0));

    assert(access == SDL_TEXTUREACCESS_STATIC || access == SDL_TEXTUREACCESS_STREAMING);
    int bytesPerPixel = SDL_BYTESPERPIXEL(m_texture->format);
    int updatedWidth = rect ? rect->w : m_texture->w;
    int pitch = bytesPerPixel * updatedWidth;

    bool isOk = false;
    if (access == SDL_TEXTUREACCESS_STATIC) {
        isOk = SDL_UpdateTexture(m_texture, rect, pixels, pitch);
    } else if (access == SDL_TEXTUREACCESS_STREAMING) {
        // https://github.com/libsdl-org/SDL/blob/c790572674b225de34fe53c0a0188dca8a05c722/src/render/ps2/SDL_render_ps2.c#L166
        void* writePixels;
        int destPitch;
        isOk = SDL_LockTexture(m_texture, rect, &writePixels, &destPitch);
        if (isOk) {
            int updatedHeight = rect ? rect->h : m_texture->h;
            if (pitch == destPitch) {
                std::memcpy(writePixels, pixels, pitch * updatedHeight);
            } else {
                uint8_t* src = (uint8_t*)pixels;
                uint8_t* dst = (uint8_t*)writePixels;
                for (int row = 0; row < updatedHeight; ++row) {
                    std::memcpy(dst, src, pitch);
                    src += pitch;
                    dst += destPitch;
                }
            }
            SDL_UnlockTexture(m_texture);
        }
    }

    if (!isOk && error) {
        *error = SDL_GetError();
    }
    return isOk;
}

void Texture::freeTexture() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}
