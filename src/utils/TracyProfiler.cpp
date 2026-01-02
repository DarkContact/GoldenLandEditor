#include "TracyProfiler.h"

#include "SDL3/SDL_render.h"

#if defined(_MSC_VER)
void* operator new(std::size_t size) {
    auto ptr = malloc(size);
    TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}
#endif

namespace TracyProfilerInternal {

void CaptureImage(SDL_Renderer* renderer) {

    SDL_Surface* screenSurface = SDL_RenderReadPixels(renderer, nullptr);

    SDL_Rect rgbaRect(0, 0, 320, 180);
    SDL_Surface* rgbaSurface = SDL_CreateSurface(rgbaRect.w, rgbaRect.h, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurfaceScaled(screenSurface, NULL, rgbaSurface, &rgbaRect, SDL_SCALEMODE_LINEAR);

    Tracy_FrameImage(rgbaSurface->pixels,
                     rgbaSurface->w,
                     rgbaSurface->h,
                     0,
                     false);

    SDL_DestroySurface(screenSurface);
    SDL_DestroySurface(rgbaSurface);

}

static SDL_malloc_func mallocFunc;
static SDL_calloc_func callocFunc;
static SDL_realloc_func reallocFunc;
static SDL_free_func freeFunc;

void* mallocSDL(size_t size) {
    auto ptr = mallocFunc(size);
    TracyAlloc(ptr, size);
    return ptr;
}

void* callocSDL(size_t nmemb, size_t size) {
    void* ptr = callocFunc(nmemb, size);
    TracyAlloc(ptr, nmemb * size);
    return ptr;
}

void* reallocSDL(void* oldPtr, size_t size) {
    TracyFree(oldPtr);
    void* ptr = reallocFunc(oldPtr, size);
    TracyAlloc(ptr, size);
    return ptr;
}

void freeSDL(void* ptr) {
    TracyFree(ptr);
    freeFunc(ptr);
}

void TrackSdlMemory() {
    SDL_GetOriginalMemoryFunctions(&mallocFunc, &callocFunc, &reallocFunc, &freeFunc);
    SDL_SetMemoryFunctions(&mallocSDL, &callocSDL, &reallocSDL, &freeSDL);
}

} // TracyProfilerInternal
