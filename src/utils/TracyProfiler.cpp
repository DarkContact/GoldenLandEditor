#include "TracyProfiler.h"

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_render.h>

#include "imgui.h"

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

// -- SDL Memory --
static SDL_malloc_func mallocFunc;
static SDL_calloc_func callocFunc;
static SDL_realloc_func reallocFunc;
static SDL_free_func freeFunc;

void* mallocSDL(size_t size) {
    auto ptr = mallocFunc(size);
    TracyAllocN(ptr, size, "SDL");
    return ptr;
}

void* callocSDL(size_t nmemb, size_t size) {
    void* ptr = callocFunc(nmemb, size);
    TracyAllocN(ptr, nmemb * size, "SDL");
    return ptr;
}

void* reallocSDL(void* oldPtr, size_t size) {
    TracyFreeN(oldPtr, "SDL");
    void* ptr = reallocFunc(oldPtr, size);
    TracyAllocN(ptr, size, "SDL");
    return ptr;
}

void freeSDL(void* ptr) {
    TracyFreeN(ptr, "SDL");
    freeFunc(ptr);
}

void TrackSdlMemory() {
    SDL_GetOriginalMemoryFunctions(&mallocFunc, &callocFunc, &reallocFunc, &freeFunc);
    SDL_SetMemoryFunctions(mallocSDL, callocSDL, reallocSDL, freeSDL);
}

// -- ImGui Memory --
void* mallocImGui(size_t size, void* /*user_data*/) {
    void* ptr = malloc(size);
    TracyAllocN(ptr, size, "ImGui");
    return ptr;
}

void freeImGui(void* ptr, void* /*user_data*/) {
    TracyFreeN(ptr, "ImGui");
    free(ptr);
}

void TrackImGuiMemory() {
    ImGui::SetAllocatorFunctions(mallocImGui, freeImGui, nullptr);
}

} // TracyProfilerInternal
