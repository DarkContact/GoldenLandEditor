#include "TextureLoader.h"

#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Texture.h"
#include "parsers/CSX_Parser.h"

#include "utils/FileUtils.h"
#include "utils/TracyProfiler.h"

bool TextureLoader::loadTextureFromMemory(std::span<const uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    int imageWidth = 0;
    int imageHeight = 0;
    int channels = 4;
    Tracy_ZoneStartN("stbImageLoad");
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> imageDataPtr = {
        stbi_load_from_memory((const stbi_uc*)memory.data(), (int)memory.size(), &imageWidth, &imageHeight, NULL, channels),
        stbi_image_free
    };
    Tracy_ZoneEnd();

    if (!imageDataPtr) {
        if (error)
            *error = std::string(stbi_failure_reason());
        return false;
    }

    Texture texture = Texture::create(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, imageWidth, imageHeight, error);
    if (!texture) {
        return false;
    }

    {
        Tracy_ZoneScopedN("updatePixels");
        if (!texture.updatePixels(imageDataPtr.get(), nullptr, error)) {
            return false;
        }
    }

    outTexture = std::move(texture);
    return true;
}

bool TextureLoader::loadTextureFromFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    return loadTextureFromMemory(fileData, renderer, outTexture, error);
}

bool TextureLoader::loadTextureFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(true, error),
        SDL_DestroySurface
    };

    if (!surfacePtr)
        return false;

    Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
    if (!texture)
        return false;

    outTexture = std::move(texture);

    return true;
}

bool TextureLoader::loadTexturesFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(true, error),
        SDL_DestroySurface
    };

    if (!surfacePtr)
        return false;

    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    int maxTextureSize = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);

    if (surfacePtr->h > maxTextureSize) {
        int fullHeight = surfacePtr->h;
        int yOffset = 0;
        while (fullHeight > 0) {
            SDL_Rect srcRect(0, yOffset, surfacePtr->w, std::min(maxTextureSize, fullHeight));

            std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> subSurfacePtr = {
                SDL_CreateSurface(srcRect.w, srcRect.h, SDL_PIXELFORMAT_INDEX8),
                SDL_DestroySurface
            };
            if (!subSurfacePtr) {
                if (error)
                    *error = std::string(SDL_GetError());
                return false;
            }

            SDL_SetSurfacePalette(subSurfacePtr.get(), SDL_GetSurfacePalette(surfacePtr.get()));

            // Восстановим прозрачность
            if (SDL_SurfaceHasColorKey(surfacePtr.get())) {
                Uint32 key = 0;
                SDL_GetSurfaceColorKey(surfacePtr.get(), &key);
                SDL_SetSurfaceColorKey(subSurfacePtr.get(), true, key);
                SDL_FillSurfaceRect(subSurfacePtr.get(), NULL, key);
            }

            SDL_BlitSurface(surfacePtr.get(), &srcRect, subSurfacePtr.get(), NULL);

            Texture texture = Texture::createFromSurface(renderer, subSurfacePtr.get(), error);
            if (!texture)
                return false;

            outTextures.push_back(std::move(texture));
            fullHeight -= srcRect.h;
            yOffset += srcRect.h;
        }
    } else {
        Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
        if (!texture)
            return false;

        outTextures.push_back(std::move(texture));
    }

    return true;
}

bool TextureLoader::loadHeightAnimationFromCsxFile(std::string_view fileName, int height, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    return loadAnimationFromCsxFile(fileName, IntParam::kHeight, height, renderer, outTextures, error);
}

bool TextureLoader::loadCountAnimationFromCsxFile(std::string_view fileName, int count, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    return loadAnimationFromCsxFile(fileName, IntParam::kCount, count, renderer, outTextures, error);
}

bool TextureLoader::loadAnimationFromCsxFile(std::string_view fileName, IntParam type, int param, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;

    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    if (!csxParser.preParse(error))
        return false;

    int height;
    assert(csxParser.metaInfo().height % param == 0);
    if (type == IntParam::kHeight) {
        height = param;
    } else if (type == IntParam::kCount) {
        height = csxParser.metaInfo().height / param;
    }

    SDL_Surface* surface = SDL_CreateSurface(csxParser.metaInfo().width, height, SDL_PIXELFORMAT_INDEX8);
    if (!surface) {
        if (error)
            *error = std::string(SDL_GetError());
        return false;
    }

    int countTextures = csxParser.metaInfo().height / height;
    for (int i = 0; i < countTextures; ++i) {
        Tracy_ZoneScopedN("Create texture");
        Tracy_ZoneTextF("%d", i);
        int lineIndexStart = i * height;
        csxParser.parseLinesToSurface(surface, true, lineIndexStart, height, true, error);
        Texture texture = Texture::createFromSurface(renderer, surface, error);
        if (!texture) {
            SDL_DestroySurface(surface);
            return false;
        }
        outTextures.push_back(std::move(texture));
    }
    SDL_DestroySurface(surface);
    return true;
}
