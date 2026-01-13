#include "RenderUtils.h"

#include <string_view>

#include <SDL3/SDL_render.h>

#ifdef _WIN32
  #include <d3d11.h>
  #include <dxgi1_4.h>
  #include <wrl/client.h>
#endif

uint64_t RenderUtils::getUsedVideoMemoryBytes(SDL_Renderer* renderer)
{
#ifdef _WIN32
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    std::string_view rendererName = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "unknown");

    if (rendererName == "direct3d11") {
        using Microsoft::WRL::ComPtr;
        ID3D11Device* device = (ID3D11Device*)SDL_GetPointerProperty(props, SDL_PROP_RENDERER_D3D11_DEVICE_POINTER, nullptr);

        if (device) {
            ComPtr<IDXGIDevice> dxgiDevice;
            device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

            ComPtr<IDXGIAdapter> adapter;
            dxgiDevice->GetAdapter(&adapter);

            ComPtr<IDXGIAdapter3> adapter3;
            adapter.As(&adapter3);

            DXGI_QUERY_VIDEO_MEMORY_INFO info{};
            adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);

            return info.CurrentUsage;
        }
    }
#endif

    return 0;
}
