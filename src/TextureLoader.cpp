#include "TextureLoader.h"

#include "imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool TextureLoader::loadTextureFromMemory(const void* data, size_t data_size, SDL_Renderer* renderer, SDL_Texture** out_texture)
{
    int image_width = 0;
    int image_height = 0;
    int channels = 4;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
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

    *out_texture = texture;

    SDL_DestroySurface(surface);
    stbi_image_free(image_data);

    return true;
}

bool TextureLoader::loadTextureFromFile(const char* file_name, SDL_Renderer* renderer, SDL_Texture** out_texture)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    bool ret = loadTextureFromMemory(file_data, file_size, renderer, out_texture);
    IM_FREE(file_data);
    return ret;
}
