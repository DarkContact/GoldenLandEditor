#include "TextureLoader.h"

#include <memory>

#include "stb_image.h"

#include "parsers/CSX_Parser.h"
#include "Texture.h"

#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/DebugLog.h"

bool TextureLoader::loadTextureFromFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    return loadTextureFromMemory(fileData, renderer, outTexture, error);
}

bool TextureLoader::loadTextureFromMemory(std::span<const uint8_t> memory, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    bool have16BitSupport = false;
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    const SDL_PixelFormat* formats = (const SDL_PixelFormat*)SDL_GetPointerProperty(props, SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, NULL);
    if (formats) {
        int i = 0;
        while (formats[i] != SDL_PIXELFORMAT_UNKNOWN) {
            if (formats[i] == SDL_PIXELFORMAT_RGB565) {
                have16BitSupport = true;
                break;
            }
            ++i;
        }
    }

    int imageWidth = 0;
    int imageHeight = 0;
    int channels = have16BitSupport ? 3 : 4;
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

    Texture texture = Texture::create(renderer,
                                      have16BitSupport ? SDL_PIXELFORMAT_RGB565
                                                       : SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STATIC,
                                      imageWidth,
                                      imageHeight,
                                      error);
    if (!texture) {
        return false;
    }

    {
        if (have16BitSupport) {
            Tracy_ZoneScopedN("convertPixels");
            const int srcPitch = 3 * imageWidth;
            const int dstPitch = 2 * imageWidth;
            bool isOk = SDL_ConvertPixels(imageWidth, imageHeight,
                                          SDL_PIXELFORMAT_RGB24, imageDataPtr.get(), srcPitch,
                                          SDL_PIXELFORMAT_RGB565, imageDataPtr.get(), dstPitch);
            if (!isOk) {
                if (error)
                    *error = std::string(SDL_GetError());
                return false;
            }
        }

        if (!texture.updatePixels(imageDataPtr.get(), nullptr, error)) {
            return false;
        }
    }

    outTexture = std::move(texture);
    return true;
}

bool TextureLoader::loadTextureFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, Texture& outTexture, std::string* error)
{
    Tracy_ZoneScoped;
    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(false, error),
        SDL_DestroySurface
    };

    if (!surfacePtr)
        return false;

    Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
    if (!texture)
        return false;

    //SDL_SetTexturePalette(texture.get(), csxParser.metaInfo().pallete);
    SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
    outTexture = std::move(texture);

    return true;
}


bool TextureLoader::loadTexturesFromCsxFile(std::string_view fileName, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    int maxTextureSize = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
    return loadAnimationFromCsxFile(fileName, IntParam::kHeight, maxTextureSize, true, renderer, outTextures, error);
}

bool TextureLoader::saveCsxAsBmpFile(std::string_view fileNameCsx, std::string_view fileNameBmp, SDL_Renderer* renderer, std::string* error)
{
    Tracy_ZoneScoped;

    std::vector<uint8_t> fileData = FileUtils::loadFile(fileNameCsx, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        csxParser.parse(false, error),
        SDL_DestroySurface
    };

    if (!surfacePtr)
        return false;

    bool isOk = SDL_SaveBMP(surfacePtr.get(), fileNameBmp.data());
    if (!isOk) {
        if (error)
            *error = SDL_GetError();
        return false;
    }

    return true;
}

bool TextureLoader::loadHeightAnimationFromCsxFile(std::string_view fileName, int height, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    return loadAnimationFromCsxFile(fileName, IntParam::kHeight, height, false, renderer, outTextures, error);
}

bool TextureLoader::loadCountAnimationFromCsxFile(std::string_view fileName, int count, SDL_Renderer* renderer, std::vector<Texture>& outTextures, std::string* error)
{
    Tracy_ZoneScoped;
    return loadAnimationFromCsxFile(fileName, IntParam::kCount, count, false, renderer, outTextures, error);
}

bool TextureLoader::loadCountAnimationFromBmpFile(std::string_view fileName, int count, SDL_Renderer* renderer, std::vector<Texture>& outTextures, const SDL_Color* transparentColor, std::string* error) {
    Tracy_ZoneScoped;

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr = {
        SDL_LoadBMP(fileName.data()),
        SDL_DestroySurface
    };

    if (!surfacePtr) {
        if (error)
            *error = SDL_GetError();
        return false;
    }

    int fullHeight = surfacePtr->h;
    int frameHeight = fullHeight / count;
    if (fullHeight % count != 0) {
        LogFmt("Warning in {}: (bmpHeight % framesCount != 0) [bmpHeight: {}, framesCount: {}, frameHeight: {}]",
               StringUtils::filename(fileName), fullHeight, count, frameHeight);
    }

    if (transparentColor) {
        Uint32 key = SDL_MapRGB(SDL_GetPixelFormatDetails(surfacePtr->format), SDL_GetSurfacePalette(surfacePtr.get()), transparentColor->r, transparentColor->g, transparentColor->b);
        SDL_SetSurfaceColorKey(surfacePtr.get(), true, key);
    }

    if (count == 1) {
        Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
        if (!texture)
            return false;

        outTextures.push_back(std::move(texture));
        return true;
    }

    int yOffset = 0;
    for (int i = 0; i < count; ++i) {
        SDL_Rect srcRect(0, yOffset, surfacePtr->w, frameHeight);

        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> subSurfacePtr = {
            SDL_CreateSurface(srcRect.w, srcRect.h, surfacePtr->format),
            SDL_DestroySurface
        };
        if (!subSurfacePtr) {
            if (error)
                *error = SDL_GetError();
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

        if (!SDL_BlitSurface(surfacePtr.get(), &srcRect, subSurfacePtr.get(), NULL)) {
            if (error)
                *error = SDL_GetError();
            return false;
        }

        Texture texture = Texture::createFromSurface(renderer, subSurfacePtr.get(), error);
        if (!texture)
            return false;

        outTextures.push_back(std::move(texture));
        yOffset += frameHeight;
    }
    return true;
}

bool TextureLoader::loadAnimationFromCsxFile(std::string_view fileName,
                                             IntParam type, int param,
                                             bool keepPartialFrame,
                                             SDL_Renderer* renderer,
                                             std::vector<Texture>& outTextures,
                                             std::string* error)
{
    Tracy_ZoneScoped;
    if (param <= 0) {
        if (error)
            *error = "Invalid param: must be > 0";
        return false;
    }

    std::vector<uint8_t> fileData = FileUtils::loadFile(fileName, error);
    if (fileData.empty())
        return false;

    CSX_Parser csxParser(fileData);
    if (!csxParser.preParse(error))
        return false;

    int frameHeight;
    bool havePartialFrame = false;
    if (type == IntParam::kHeight) {
        frameHeight = param;
        if (csxParser.metaInfo().height % param != 0) {
            if (!keepPartialFrame) {
                LogFmt("Warning in {}: (csxHeight % frameHeight != 0) [csxHeight: {}, framesCount: {}, frameHeight: {}]",
                       StringUtils::filename(fileName), csxParser.metaInfo().height, csxParser.metaInfo().height / param, frameHeight);
            }
            havePartialFrame = true;
        }
    } else if (type == IntParam::kCount) {
        frameHeight = csxParser.metaInfo().height / param;
        if (csxParser.metaInfo().height % param != 0) {
            if (!keepPartialFrame) {
                LogFmt("Warning in {}: (csxHeight % framesCount != 0) [csxHeight: {}, framesCount: {}, frameHeight: {}]",
                       StringUtils::filename(fileName), csxParser.metaInfo().height, param, frameHeight);
            }
            havePartialFrame = true;
        }
    }

    // Кадры одинаковой высоты
    int countTextures = csxParser.metaInfo().height / frameHeight;
    if (countTextures > 0) {
        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surfacePtr{
            SDL_CreateSurface(csxParser.metaInfo().width, frameHeight, SDL_PIXELFORMAT_INDEX8),
            SDL_DestroySurface
        };

        if (!surfacePtr) {
            if (error)
                *error = SDL_GetError();
            return false;
        }

        for (int i = 0; i < countTextures; ++i) {
            Tracy_ZoneScopedN("Create texture");
            Tracy_ZoneTextF("%d", i);
            int lineIndexStart = i * frameHeight;
            csxParser.parseLinesToSurface(surfacePtr.get(), true, lineIndexStart, frameHeight, false, error);
            Texture texture = Texture::createFromSurface(renderer, surfacePtr.get(), error);
            if (!texture) {
                return false;
            }

            //SDL_SetTexturePalette(texture.get(), csxParser.metaInfo().pallete);
            SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
            outTextures.push_back(std::move(texture));
        }
    }

    // Если высота не делится нацело добавим ещё один кадр с остатком этой высоты
    if (keepPartialFrame && havePartialFrame) {
        int lineIndexStart = countTextures * frameHeight;
        int partialHeight = csxParser.metaInfo().height - lineIndexStart;

        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> partSurfacePtr = {
            SDL_CreateSurface(csxParser.metaInfo().width, partialHeight, SDL_PIXELFORMAT_INDEX8),
            SDL_DestroySurface
        };

        if (!partSurfacePtr) {
            if (error)
                *error = SDL_GetError();
            return false;
        }

        csxParser.parseLinesToSurface(partSurfacePtr.get(), true, lineIndexStart, partialHeight, false, error);
        Texture texture = Texture::createFromSurface(renderer, partSurfacePtr.get(), error);
        if (!texture) {
            return false;
        }

        //SDL_SetTexturePalette(texture.get(), csxParser.metaInfo().pallete);
        SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
        outTextures.push_back(std::move(texture));
    }
    return true;
}
