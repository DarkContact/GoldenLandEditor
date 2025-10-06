#pragma once
#include <span>
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>

class Texture;
struct SDL_Renderer;

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(std::span<const uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTextureFromFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTextureFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTexturesFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error = nullptr);
};
