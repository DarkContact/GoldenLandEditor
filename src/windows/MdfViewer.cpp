#include "MdfViewer.h"

#include <format>
#include <array>

#include "SDL3/SDL_render.h"
#include "imgui.h"

#include "files/BgMdfFile.h"
#include "parsers/MDF_Parser.h"
#include "utils/AnimationCachedLoader.h"
#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"
#include "Types.h"

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

static std::string mdfParamsString(const MDF_Params& params) {
    return std::format("[p01]: {} [p02]: {} [flags]: {} [nFrame]: {} [p05]: {}\n"
                       "[delay]: {:.2f} [alpha]: {:.2f} [p08]: {:.2f} [p09]: {:.2f} [ms]: {}",
                       params.p01, params.p02, params.flags, params.nFrame, params.p05,
                       params.delayMs, params.alpha, params.p08, params.p09, params.animationTimeMs);
}

static std::string mdfAnimationString(const MDF_Animation& anim) {
    std::string params;
    int paramIndex = 0;
    for (const auto& param : anim.params) {
        auto paramInfo = mdfParamsString(param);
        params += std::format("PARAMS {}\n"
                              "{}\n",
                              paramIndex + 1,
                              paramInfo);
        ++paramIndex;
    }

    return std::format("[frames]: {} [xOff]: {} [yOff]: {} [a04]: {}\n"
                       "[reverse]: {} [startMs]: {} [endMs]: {}\n"
                       "{}"
                       "animationPath: {}\n"
                       "{}",
                       anim.framesCount, anim.xOffset, anim.yOffset, anim.a04,
                       anim.isReverse, anim.startTimeMs, anim.endTimeMs,
                       anim.maskAnimationPath.empty() ? std::string{}
                                                      : std::format("maskAnimationPath: {}\n", anim.maskAnimationPath),
                       anim.animationPath,
                       params);
}

static std::string mdfInfoString(const MDF_Data& data, const std::vector<LayerInfo>& layerInfos) {
    assert(data.layers.size() == layerInfos.size());

    std::string result = std::format("totalDurationMs: {}\n\n", data.totalDurationMs);
    for (int l = 0; l < data.layers.size(); ++l) {
        const auto& layer = data.layers[l];
        const auto& layerInfo = layerInfos[l];
        assert(layer.animations.size() == layerInfo.animationInfo.size());
        result += std::format("LAYER {} [num: {}]\n", l + 1, layer.layerNumber);
        for (int a = 0; a < layer.animations.size(); ++a) {
            const auto& animation = layer.animations[a];
            const auto& animationInfo = layerInfo.animationInfo[a];
            result += std::format("ANIMATION {} ({}x{})\n", a + 1, animationInfo.width, animationInfo.height);
            result += mdfAnimationString(animation);
            result += "\n";
        }
        result += "\n";
    }
    result = StringUtils::trimRight(result);
    return result;
}

void MdfViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& mdfFiles)
{
    Tracy_ZoneScoped;

    static int selectedIndex = -1;
    static AnimationCachedLoader animationLoader;
    static std::vector<std::vector<MagicAnimation>> animationLayers;
    static std::vector<LayerInfo> layerInfos;
    static std::string mdfDataInfo;
    static std::string uiError;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static Texture bgTexture;
    static bool showBgTexture = false;
    static int activeButtonIndex = 0;
    static bool showInfo = false;
    static bool showCenter = false;
    static bool playAnimation = true;
    static int animationCurrentTime = 0;
    static ImGuiTextFilter textFilter;
    static bool onceWhenOpen = false;

    bool needResetScroll = false;

    if (!onceWhenOpen) {
        std::string error;
        if (!TextureLoader::loadTextureFromMemory({bg_mdf, bg_mdf_size}, renderer, bgTexture, &error))
            LogFmt("Load bgTexture error: {}", error);
        onceWhenOpen = true;
    }

    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
    ImGui::Begin("MDF Viewer", &showWindow);

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        textFilter.Draw();
        ImGui::Separator();
        ImGui::BeginChild("file list");
        for (int i = 0; i < static_cast<int>(mdfFiles.size()); ++i)
        {
            if (textFilter.PassFilter(mdfFiles[i].c_str())
                && ImGui::Selectable(mdfFiles[i].c_str(), selectedIndex == i))
            {
                selectedIndex = i;

                animationCurrentTime = 0;
                animationLoader.clear();
                uiError.clear();
                mdfDataInfo.clear();
                animationLayers.clear();
                layerInfos.clear();

                auto mdfDataOpt = MDF_Parser::parse(std::format("{}/{}", rootDirectory, mdfFiles[i]), &uiError);
                if (mdfDataOpt) {
                    auto& mdfData = *mdfDataOpt;

                    animationLayers.resize(mdfData.layers.size());
                    layerInfos.resize(mdfData.layers.size());

                    int layerIndex = 0;
                    for (const auto& layerDesc : mdfData.layers) {
                        auto& animationLayer = animationLayers[layerIndex];
                        animationLayer.resize(layerDesc.animations.size());

                        auto& layerInfo = layerInfos[layerIndex];
                        layerInfo.count = layerDesc.animations.size();

                        layerInfo.animationInfo.resize(layerInfo.count);

                        int animationIndex = 0;
                        for (const auto& animDesc : layerDesc.animations) {
                            auto& animation = animationLayer[animationIndex];
                            animation.setTimes(animDesc.params.front().delayMs,
                                               animDesc.startTimeMs,
                                               animDesc.endTimeMs,
                                               mdfData.totalDurationMs,
                                               animDesc.isReverse == 1);

                            animation.xOffset = animDesc.xOffset;
                            animation.yOffset = animDesc.yOffset;
                            animation.flags = animDesc.params.front().flags;
                            animation.alphaValue = animDesc.params.front().alpha * 255.0f;

                            std::string animationPath = std::format("{}/magic/bitmap/{}", rootDirectory, animDesc.animationPath);
                            SDL_Color transparentColor = {255, 0, 255, 255};
                            auto textures = animationLoader.loadAnimationCount(animationPath,
                                                                               animDesc.framesCount,
                                                                               renderer,
                                                                               (animDesc.maskAnimationPath.empty() ? &transparentColor : nullptr),
                                                                               &uiError);
                            animation.setTextures(textures);

                            layerInfo.animationInfo[animationIndex].width = animation.width();
                            layerInfo.animationInfo[animationIndex].height = animation.height();
                            layerInfo.maxWidth = std::max(animation.width(), layerInfo.maxWidth);
                            layerInfo.maxHeight = std::max(animation.height(), layerInfo.maxHeight);

                            if (textures.empty()) break;
                            ++animationIndex;
                        }
                        ++layerIndex;
                    }

                    mdfDataInfo = mdfInfoString(mdfData, layerInfos);
                }

                needResetScroll = true;
            }
        }
        ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Right
    int animationMaxTime = 0;
    if (!animationLayers.empty() && uiError.empty()) {
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 3), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            if (showBgTexture) {
                ImVec2 startPos = ImGui::GetCursorScreenPos();
                ImGui::Image((ImTextureID)bgTexture.get(), ImVec2(bgTexture->w, bgTexture->h));
                ImGui::SetCursorScreenPos(startPos);
            }

            int maxTextureW = 0;
            int maxTextureH = 0;
            int maxTextureXOffset = 0;
            for (const auto& layerInfo : layerInfos) {
                maxTextureW = std::max(layerInfo.maxWidth, maxTextureW);
                maxTextureH = std::max(layerInfo.maxHeight, maxTextureH);
            }

            int centerW = maxTextureW / 2;
            int centerH = maxTextureH / 2;

            // Коррекция отображения чтобы те текстуры которые ушли за {0,0} можно было показать полностью
            int minX = 0;
            int minY = 0;
            for (auto& layer : animationLayers) {
                for (auto& animation : layer) {
                    int animPosX = centerW - (animation.width() / 2) + animation.xOffset;
                    int animPosY = centerH - (animation.height() / 2) + animation.yOffset;
                    minX = std::min(animPosX, minX);
                    minY = std::min(animPosY, minY);
                }
            }

            ImVec2 startPos = ImGui::GetCursorScreenPos();
            for (auto& layer : animationLayers) {
                for (auto& animation : layer) {
                    animationMaxTime = animation.getTotalDurationMs();

                    animation.update(animationCurrentTime);
                    if (animation.isActive()) {
                        const Texture& currentTexture = animation.currentTexture();
                        int animPosX = centerW - (currentTexture->w / 2);
                        int animPosY = centerH - (currentTexture->h / 2);
                        const ImVec2 animationPos{startPos.x + animPosX + animation.xOffset - minX, startPos.y + animPosY + animation.yOffset - minY};
                        ImGui::SetCursorScreenPos(animationPos);

                        SDL_SetTextureBlendMode(currentTexture.get(), animation.blendMode());
                        ImVec4 tintColor{1, 1, 1, animation.alpha() / 255.0f};
                        ImGui::ImageWithBg((ImTextureID)currentTexture.get(),
                                           ImVec2(currentTexture->w, currentTexture->h),
                                           ImVec2(0, 0), ImVec2(1, 1), bgColor, tintColor);
                    }
                    maxTextureXOffset = std::max(animation.xOffset, maxTextureXOffset);
                }
            }

            if (showCenter) {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddCircleFilled({startPos.x + centerW - minX, startPos.y + centerH - minY}, 2.0f, IM_COL32(255, 0, 0, 255));
            }

            if (showInfo) {
                ImGui::SetCursorScreenPos({startPos.x + maxTextureW + maxTextureXOffset + 12.0f, startPos.y});

                ImGui::PushFont(NULL, 14.0f);
                ImGui::Text("%s", mdfDataInfo.c_str());
                ImGui::PopFont();
            }

            ImGui::EndChild();

            ImGui::Checkbox("Play", &playAnimation);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(400);

            ImGui::BeginDisabled(playAnimation);
            ImGui::SliderInt("Time", &animationCurrentTime, 0, animationMaxTime - 1, "%d ms", ImGuiSliderFlags_AlwaysClamp);
            ImGui::EndDisabled();

            // Информация о слоях
            {
                std::array<char, 256> entries;
                int offset = 0;
                for (int i = 0; i < animationLayers.size(); ++i) {
                    auto result = std::format_to_n(entries.data() + offset, entries.size() - offset, "L{} = {} ({}x{}), ", i + 1,
                                                   layerInfos[i].count, layerInfos[i].maxWidth, layerInfos[i].maxHeight);
                    offset += result.size;
                }
                entries[offset - 2] = '\0';

                std::array<char, 300> entriesMessage;
                auto resultFmt = std::format_to_n(entriesMessage.data(), entriesMessage.size() - 1, "Animations count: [{}]", entries.data());
                entriesMessage[std::min((size_t)resultFmt.size, entriesMessage.size() - 1)] = '\0';
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f) ,"%s", entriesMessage.data());
            }

            if (ImGui::RadioButton("Transparent", activeButtonIndex == 0)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
                activeButtonIndex = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Black", activeButtonIndex == 1)) {
                bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                activeButtonIndex = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Gray", activeButtonIndex == 2)) {
                bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                activeButtonIndex = 2;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("White", activeButtonIndex == 3)) {
                bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                activeButtonIndex = 3;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Info", &showInfo);
            ImGui::SameLine();
            ImGui::Checkbox("Center", &showCenter);
            ImGui::SameLine();
            ImGui::Checkbox("Background", &showBgTexture);
        }
        ImGui::EndGroup();
    } else if (selectedIndex >= 0 && !uiError.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", uiError.c_str());
    }

    ImGui::End();

    if (playAnimation) {
        animationCurrentTime += ImGui::GetIO().DeltaTime * 1000;
        if (animationCurrentTime >= animationMaxTime) {
            animationCurrentTime -= animationMaxTime;

            if (animationCurrentTime < 0 || animationCurrentTime >= animationMaxTime) {
                animationCurrentTime = 0;
            }
        }
    }

    if (!showWindow) {
        selectedIndex = -1;
        animationLoader.clear();
        animationCurrentTime = 0;
        animationLayers.clear();
        layerInfos.clear();
        uiError.clear();
        textFilter.Clear();
        bgTexture = {};
        onceWhenOpen = false;
    }
}
