#include "TextureLoader.h"

#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Texture.h"
#include "parsers/CSX_Parser.h"

#include "utils/FileLoader.h"
#include "utils/TracyProfiler.h"

bool TextureLoader::loadTextureFromMemory(std::span<uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    int imageWidth = 0;
    int imageHeight = 0;
    int channels = 4;
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> imageDataPtr = {
        stbi_load_from_memory((const stbi_uc*)memory.data(), (int)memory.size(), &imageWidth, &imageHeight, NULL, channels),
        stbi_image_free
    };

    if (!imageDataPtr) {
        if (error)
            *error = std::string(stbi_failure_reason());
        return false;
    }

    Texture texture = Texture::create(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, imageWidth, imageHeight, error);
    if (!texture) {
        return false;
    }

    if (!texture.updatePixels(imageDataPtr.get(), nullptr, error)) {
        return false;
    }

    outTexture = std::move(texture);
    return true;
}

bool TextureLoader::loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    return loadTextureFromMemory(fileData, renderer, outTexture, error);
}

bool TextureLoader::loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(),
        SDL_DestroySurface
    };

    if (!surfacePtr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError()); // FIXME: Ошибка должна придти из парсера
        return false;
    }

    // FIXME: Грузить текстуру вне зависимости от размера поверхности
    Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
    if (!texture) {
        return false;
    }

    outTexture = std::move(texture);
    return true;
}
