#pragma once

#ifdef TRACY_ENABLE
    #include "tracy/Tracy.hpp"

    #define Tracy_ZoneScoped ZoneScoped
    #define Tracy_ZoneScopedN(name) ZoneScopedN(name)

    #define Tracy_FrameImage(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)

    #define Tracy_FrameMark FrameMark

#else
    #define Tracy_ZoneScoped
    #define Tracy_ZoneScopedN(name)

    #define Tracy_FrameImage(image, width, height, offset, flip)

    #define Tracy_FrameMark
#endif

// Работает, но очень медленно (15 мс)
// {
//     int screenW, screenH;
//     SDL_GetRenderOutputSize(renderer, &screenW, &screenH);

//     SDL_Rect rect;
//     rect.w = 320;
//     rect.h = 180;
//     rect.x = (screenW - rect.w) / 2;
//     rect.y = (screenH - rect.h) / 2;

//     SDL_Surface* surface = SDL_RenderReadPixels(renderer, &rect);
//     SDL_Surface* rgbaSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);

//     Tracy_FrameImage(rgbaSurface->pixels,
//                      rgbaSurface->w,
//                      rgbaSurface->h,
//                      0,
//                      false);

//     SDL_DestroySurface(surface);
//     SDL_DestroySurface(rgbaSurface);
// }
