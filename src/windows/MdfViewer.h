#pragma once
#include <vector>
#include <string>

#include <SDL3/SDL_blendmode.h>
#include "imgui.h"

#include "graphics/TimedAnimation.h"
#include "graphics/Texture.h"
#include "parsers/MDF_Parser.h"
#include "Cache.h"

struct SDL_Renderer;

class MdfViewer {
public:
    MdfViewer();

    void update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& mdfFiles);
    bool isAnimating() const;

private:
    struct AnimationInfo {
        int width = -1;
        int height = -1;
    };

    struct LayerInfo {
        int count = -1;
        int maxWidth = -1;
        int maxHeight = -1;
        std::vector<AnimationInfo> animationInfo;
    };

    struct MagicAnimation : public TimedAnimation {
        int xOffset;
        int yOffset;
        int32_t flags;
        Uint8 alphaValue;

        Uint32 blendMode() {
            if (flags == 256) {
                // elem_stoneheadshot
                // gods_earth_shaking gods_roy_insect gods_tucha_moshki
                // shad_shadowcower shad_tenewoerasseyanie shad_tenewoerasseyanie2
                // vis_dark
                SDL_BlendMode invertAdditiveBlend = SDL_ComposeCustomBlendMode(
                    SDL_BLENDFACTOR_SRC_ALPHA,
                    SDL_BLENDFACTOR_ONE,
                    SDL_BLENDOPERATION_REV_SUBTRACT,
                    SDL_BLENDFACTOR_ZERO,
                    SDL_BLENDFACTOR_ONE,
                    SDL_BLENDOPERATION_ADD);
                return invertAdditiveBlend; // +
            }
            if (flags == 128) return SDL_BLENDMODE_ADD; // +
            if (flags == 64) return SDL_BLENDMODE_ADD; // Похоже на эффект overlay но не оно (GoldArrowRain, ManyashiiOgonek, Firestorm)
            if (flags == 32) return SDL_BLENDMODE_BLEND; // +
            if (flags == 16) return SDL_BLENDMODE_BLEND; // +
            if (flags == 8) return SDL_BLENDMODE_BLEND; // +
            return SDL_BLENDMODE_BLEND;
        }

        Uint8 alpha() {
            if (flags == 256) return 255; // +
            if (flags == 128) return 255; // +
            if (flags == 64) return 64; // До конца непонятно что за режим, потому эмулируем его через полупрозрачность
            if (flags == 32) return 255; // Вероятно обозначает наличие маски (lght_sanctity)
            if (flags == 16) return alphaValue;  // +
            if (flags == 8) return 255;   // +
            return 255;
        }
    };

    std::string mdfParamsString(const MDF_Params& params);
    std::string mdfAnimationString(const MDF_Animation& anim);
    std::string mdfInfoString(const MDF_Data& data);

    int m_selectedIndex = -1;
    Cache<std::vector<Texture>> m_animationLoader;
    std::vector<std::vector<MagicAnimation>> m_animationLayers;
    std::vector<LayerInfo> m_layerInfos;
    std::string m_mdfDataInfo;
    std::string m_uiError;
    ImVec4 m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    Texture m_bgTexture;
    bool m_showBgTexture = false;
    int m_activeButtonIndex = 0;
    bool m_showInfo = false;
    bool m_showCenter = false;
    bool m_playAnimation = true;
    int m_animationCurrentTime = 0;
    ImGuiTextFilter m_textFilter;
    bool m_onceWhenOpen = false;
    bool m_onceWhenClose = true;
};
