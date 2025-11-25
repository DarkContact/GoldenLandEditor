#pragma once
#include <vector>
#include <string>
#include <span>

#include <SDL3/SDL_blendmode.h>
#include "imgui.h"

#include "Texture.h"
#include "parsers/MDF_Parser.h"
#include "utils/AnimationCachedLoader.h"

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

    struct TimedAnimation {

        int width() const { return this->textures.front()->w; }
        int height() const { return this->textures.front()->h; }

        void setTextures(std::span<const Texture> textures) {
            this->textures = textures;
            assert(!this->textures.empty());
        }

        const Texture& currentTexture() const {
            assert(currentTimeMs >= 0 && currentTimeMs < totalDurationMs);
            int correctedTime = currentTimeMs - startTimeMs;
            int currentFrame = correctedTime / delayMs;

            if (currentFrame >= textures.size()) {
                currentFrame = textures.size() - 1;
            }

            // dark_boiling_blood
            // dark_sendlife
            // elem_oledinenie
            if (isReverse) {
                currentFrame = textures.size() - 1 - currentFrame;

                if (currentFrame < 0) {
                    currentFrame = 0;
                }
            }

            return textures[currentFrame];
        }

        bool isActive() const { return active; }

        void update(uint64_t currentTimeMs) {
            assert(totalDurationMs > 0);

            this->currentTimeMs = currentTimeMs;
            active = (currentTimeMs >= startTimeMs && currentTimeMs < endTimeMs);
        }

        void setTimes(float delayMs, uint64_t startTimeMs, uint64_t endTimeMs, uint64_t totalDurationMs, bool isReverse) {
            this->delayMs = delayMs;
            this->startTimeMs = startTimeMs;
            this->endTimeMs = endTimeMs;
            this->totalDurationMs = totalDurationMs;
            this->isReverse = isReverse;
        }

        uint64_t getTotalDurationMs() const {
            return totalDurationMs;
        }

    private:
        std::span<const Texture> textures;
        float delayMs = 0;
        uint64_t startTimeMs = 0;
        uint64_t endTimeMs = 0;
        uint64_t totalDurationMs = 0;
        bool isReverse = false;

        uint64_t currentTimeMs = 0;
        bool active = false;
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
    AnimationCachedLoader m_animationLoader;
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
