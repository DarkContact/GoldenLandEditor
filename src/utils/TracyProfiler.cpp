#include "TracyProfiler.h"

#include "SDL3/SDL_render.h"

#if defined(_MSC_VER)
void* operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
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

} // TracyProfilerInternal
