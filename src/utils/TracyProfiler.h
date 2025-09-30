#pragma once

#ifdef TRACY_ENABLE
    #include "tracy/Tracy.hpp"

    #define Tracy_ZoneScoped ZoneScoped
    #define Tracy_ZoneScopedN(name) ZoneScopedN(name)

    #define Tracy_ZoneText(txt, size) ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...) ZoneTextF(fmt, ##__VA_ARGS__)
    #define Tracy_ZoneColor(color) ZoneColor(color)

    #define Tracy_FrameImage(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)

    #define Tracy_FrameMark FrameMark

    #include "SDL3/SDL_render.h"
    void CaptureImage(SDL_Renderer* renderer); // Работает, но медленно (~12-15 мс)

#else
    #define Tracy_ZoneScoped
    #define Tracy_ZoneScopedN(name)

    #define Tracy_ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...)
    #define Tracy_ZoneColor(color)

    #define Tracy_FrameImage(image, width, height, offset, flip)

    #define Tracy_FrameMark

    struct SDL_Renderer;
    void CaptureImage(SDL_Renderer* renderer);
#endif
