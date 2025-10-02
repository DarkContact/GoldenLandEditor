#include "TextureLoader.h"

#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Texture.h"
#include "parsers/CSX_Parser.h"

#include "utils/FileLoader.h"
#include "utils/TracyProfiler.h"

bool TextureLoader::loadTextureFromMemory(std::span<uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture)
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
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return false;
    }

    std::string error;
    Texture texture = Texture::create(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, imageWidth, imageHeight, &error);
    if (!texture) {
        fprintf(stderr, "Failed to create texture: %s\n", error.c_str());
        return false;
    }

    if (!texture.updatePixels(imageDataPtr.get(), nullptr, &error)) {
        fprintf(stderr, "Failed to update texture: %s\n", error.c_str());
        return false;
    }

    outTexture = std::move(texture);
    return true;
}

bool TextureLoader::loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName);
    return loadTextureFromMemory(fileData, renderer, outTexture);
}

bool TextureLoader::loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, Texture& outTexture)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName);

    CSX_Parser csxParser(fileData.data(), fileData.size());
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(),
        SDL_DestroySurface
    };

    if (!surfacePtr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    Texture texture = Texture::createFromSurface(renderer, surfacePtr.get());
    if (!texture) {
        // FIXME: Грузить текстуру вне зависимости от размера поверхности
        fprintf(stderr, "Failed to create SDL texture %s: %s\n", fileName, SDL_GetError());
        fprintf(stderr, "Surface dimensions: %dx%d\n", surfacePtr->w, surfacePtr->h);
        return false;
    }

    outTexture = std::move(texture);
    return true;
}
