#pragma once

#ifdef TRACY_ENABLE
    #include "tracy/Tracy.hpp"

    #define Tracy_ZoneScoped ZoneScoped
    #define Tracy_ZoneScopedN(name) ZoneScopedN(name)

    #define Tracy_ZoneText(txt, size) ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...) ZoneTextF(fmt, ##__VA_ARGS__)
    #define Tracy_ZoneColor(color) ZoneColor(color)

    #define Tracy_Message(txt, size) TracyMessage(txt, size)
    #define Tracy_MessageC(txt, size, color) TracyMessageC(txt, size, color)

    #define Tracy_FrameMark FrameMark
    #define Tracy_FrameImage(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)

    #if defined(_MSC_VER)
        void* operator new(std::size_t count);
        void operator delete(void* ptr) noexcept;
    #endif

    #include "SDL3/SDL_render.h"
    void CaptureImage(SDL_Renderer* renderer); // Работает, но медленно (~12-15 мс)

#else
    #define Tracy_ZoneScoped
    #define Tracy_ZoneScopedN(name)

    #define Tracy_ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...)
    #define Tracy_ZoneColor(color)

    #define Tracy_Message(txt, size)
    #define Tracy_MessageC(txt, size, color)

    #define Tracy_FrameMark
    #define Tracy_FrameImage(image, width, height, offset, flip) 

    struct SDL_Renderer;
    void CaptureImage(SDL_Renderer* renderer);
#endif
