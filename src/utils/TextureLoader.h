#pragma once
#include <span>
#include <string>
#include <cstdint>

class Texture;
struct SDL_Renderer;

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(std::span<const uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
};
