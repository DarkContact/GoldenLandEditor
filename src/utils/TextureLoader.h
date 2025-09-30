#pragma once
#include <cstdint>

struct SDL_Renderer;
struct SDL_Texture;

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(const void* data, size_t dataSize, SDL_Renderer* renderer, SDL_Texture** outTexture);
    static bool loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, SDL_Texture** outTexture);
    static bool loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, SDL_Texture** outTexture);
};
