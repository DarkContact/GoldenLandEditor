#pragma once
#include <span>
#include <cstdint>

class Texture;
struct SDL_Renderer;

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(std::span<uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture);
    static bool loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture);
    static bool loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture);
};
