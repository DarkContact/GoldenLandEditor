#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "parsers/CSX_Parser.h"

#include "utils/FileLoader.h"
#include "utils/TracyProfiler.h"

bool TextureLoader::loadTextureFromMemory(const void* data, size_t dataSize, SDL_Renderer* renderer, SDL_Texture** outTexture)
{
    Tracy_ZoneScoped;
    int image_width = 0;
    int image_height = 0;
    int channels = 4;
    unsigned char* image_data = stbi_load_from_memory((const stbi_uc*)data, (int)dataSize, &image_width, &image_height, NULL, 4);
    if (image_data == nullptr)
    {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return false;
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(image_width, image_height, SDL_PIXELFORMAT_RGBA32, (void*)image_data, channels * image_width);
    if (surface == nullptr)
    {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr)
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());

    *outTexture = texture;

    SDL_DestroySurface(surface);
    stbi_image_free(image_data);

    return true;
}

bool TextureLoader::loadTextureFromFile(const char* fileName, SDL_Renderer* renderer, SDL_Texture** outTexture)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName);
    return loadTextureFromMemory(fileData.data(), fileData.size(), renderer, outTexture);
}

bool TextureLoader::loadTextureFromCsxFile(const char* fileName, SDL_Renderer* renderer, SDL_Texture** outTexture)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileLoader::loadFile(fileName);

    CSX_Parser csxParser(fileData.data(), fileData.size());
    SDL_Surface* surface = csxParser.parse();

    if (surface == nullptr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        fprintf(stderr, "Failed to create SDL texture %s: %s\n", fileName, SDL_GetError());
        fprintf(stderr, "Surface dimensions: %dx%d\n", surface->w, surface->h);
    }

    *outTexture = texture;

    SDL_DestroySurface(surface);
    return true;
}
