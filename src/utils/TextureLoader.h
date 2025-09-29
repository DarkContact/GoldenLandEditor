#pragma once
#include "stb_image.h"
#include "SDL3/SDL_render.h"

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(const void* data, size_t data_size, SDL_Renderer* renderer, SDL_Texture** out_texture);
    static bool loadTextureFromFile(const char* file_name, SDL_Renderer* renderer, SDL_Texture** out_texture);
    static bool loadTextureFromCsxFile(const char* file_name, SDL_Renderer* renderer, SDL_Texture** out_texture);
};
