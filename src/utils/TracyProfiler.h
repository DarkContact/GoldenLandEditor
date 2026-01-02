#pragma once

#ifdef TRACY_ENABLE
    #include "tracy/Tracy.hpp"
    #include "tracy/TracyC.h"

    #if defined(_MSC_VER)
        void* operator new(std::size_t size);
        void operator delete(void* ptr) noexcept;
    #endif

    #define Tracy_ZoneScoped ZoneScoped
    #define Tracy_ZoneScopedN(name) ZoneScopedN(name)

    #define Tracy_ZoneStartN(name) TracyCZoneN(ctx, name, true)
    #define Tracy_ZoneEnd() TracyCZoneEnd(ctx)

    #define Tracy_ZoneText(txt, size) ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...) ZoneTextF(fmt, ##__VA_ARGS__)
    #define Tracy_ZoneColor(color) ZoneColor(color)

    #define Tracy_Message(txt, size) TracyMessage(txt, size)
    #define Tracy_MessageC(txt, size, color) TracyMessageC(txt, size, color)

    #define Tracy_FrameMark FrameMark
    #define Tracy_FrameImage(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)

    #define Tracy_CaptureImage(renderer) TracyProfilerInternal::CaptureImage(renderer)
    #define Tracy_HookSdlMemory() TracyProfilerInternal::TrackSdlMemory()
    #define Tracy_HookImGuiMemory() TracyProfilerInternal::TrackImGuiMemory()

    struct SDL_Renderer;
    namespace TracyProfilerInternal {
        void CaptureImage(SDL_Renderer* renderer);
        void TrackSdlMemory();
        void TrackImGuiMemory();
    }
#else
    #define Tracy_ZoneScoped
    #define Tracy_ZoneScopedN(name)

    #define Tracy_ZoneStartN(name)
    #define Tracy_ZoneEnd()

    #define Tracy_ZoneText(txt, size)
    #define Tracy_ZoneTextF(fmt, ...)
    #define Tracy_ZoneColor(color)

    #define Tracy_Message(txt, size)
    #define Tracy_MessageC(txt, size, color)

    #define Tracy_FrameMark
    #define Tracy_FrameImage(image, width, height, offset, flip)

    #define Tracy_CaptureImage(renderer)
    #define Tracy_HookSdlMemory()
    #define Tracy_HookImGuiMemory()
#endif
