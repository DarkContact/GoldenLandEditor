#pragma once
#include <span>
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>

class Texture;
struct SDL_Renderer;
struct SDL_Color;

class TextureLoader {
public:
    TextureLoader() = delete;

    static bool loadTextureFromMemory(std::span<const uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTextureFromFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);

    static bool loadTextureFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error = nullptr);
    static bool loadTexturesFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error = nullptr); // Для огромных текстур, разбивает их по высоте
    static bool saveCsxAsBmpFile(std::string_view fileNameCsx, std::string_view fileNameBmp, SDL_Renderer* renderer, std::string* error = nullptr);

    static bool loadHeightAnimationFromCsxFile(std::string_view fileName, int height, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error = nullptr);
    static bool loadCountAnimationFromCsxFile(std::string_view fileName, int count, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error = nullptr);
    static bool loadCountAnimationFromBmpFile(std::string_view fileName, int count, SDL_Renderer* renderer, std::vector<Texture>& outTextures, const SDL_Color* transparentColor, std::string* error = nullptr);

private:
    enum class IntParam {
        kHeight,
        kCount
    };
    static bool loadAnimationFromCsxFile(std::string_view fileName, IntParam type, int param, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error);
};
