#include "MdfViewer.h"

#include <format>
#include <array>

#include "SDL3/SDL_render.h"
#include "imgui.h"

#include "parsers/MDF_Parser.h"
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

struct MagicAnimation : public BaseAnimation {
    int xOffset;
    int yOffset;
    int32_t flags;

    /*SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA,
                                              SDL_BLENDFACTOR_ONE,
                                              SDL_BLENDOPERATION_ADD,
                                              SDL_BLENDFACTOR_ZERO,
                                              SDL_BLENDFACTOR_ONE,
                                              SDL_BLENDOPERATION_ADD)*/
    Uint32 blendMode() {
        if (flags == 128) return SDL_BLENDMODE_ADD;
        if (flags == 64) return SDL_BLENDMODE_ADD;
        return SDL_BLENDMODE_BLEND;
    }

    Uint8 alpha() {
        if (flags == 128) return 255;
        if (flags == 64) return 64;
        return 255;
    }
};

static std::string mdfParamsString(const MDF_Params& params) {
    return std::format("[p01]: {} [p02]: {} [flags]: {} [nFrame]: {} [p05]: {}\n"
                       "[p06]: {:.1f} [p07]: {:.1f} [p08]: {:.1f} [p09]: {:.1f} [ms]: {}",
                       params.p01, params.p02, params.flags, params.nFrame, params.p05,
                       params.p06, params.p07, params.p08, params.p09, params.animationTimeMs);
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

    return std::format("frames: {}\n"
                       "[xOff]: {} [yOff]: {} [a04]: {}\n"
                       "[a05]: {} [startMs]: {} [endMs]: {}\n"
                       "{}"
                       "animationPath: {}\n"
                       "{}",
                       anim.framesCount,
                       anim.xOffset, anim.yOffset, anim.a04,
                       anim.a05, anim.startTimeMs, anim.endTimeMs,
                       anim.maskAnimationPath.empty() ? std::string{}
                                                      : std::format("maskAnimationPath: {}\n", anim.maskAnimationPath),
                       anim.animationPath,
                       params);
}

static std::string mdfInfoString(const MDF_Data& data, const std::vector<LayerInfo>& layerInfos) {
    assert(data.layers.size() == layerInfos.size());

    std::string result = std::format("endTimeMs: {}\n\n", data.endTimeMs);
    for (int l = 0; l < data.layers.size(); ++l) {
        const auto& layer = data.layers[l];
        const auto& layerInfo = layerInfos[l];
        assert(layer.animations.size() == layerInfo.animationInfo.size());
        result += std::format("LAYER {}\n", l + 1);
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
    static std::vector<std::vector<MagicAnimation>> animationLayers;
    static std::vector<LayerInfo> layerInfos;
    static std::string mdfDataInfo;
    static std::string uiError;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static int activeButtonIndex = 0;
    static bool showInfo = false;
    static bool showCenter = false;
    static ImGuiTextFilter textFilter;

    bool needResetScroll = false;

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
                            animation.delayMs = (animDesc.endTimeMs - animDesc.startTimeMs) / animDesc.framesCount;
                            animation.xOffset = animDesc.xOffset;
                            animation.yOffset = animDesc.yOffset;
                            animation.flags = animDesc.params.front().flags;

                            bool isOk = false;
                            if (animDesc.animationPath.ends_with(".bmp")) {
                                SDL_Color transparentColor = {255, 0, 255, 255};
                                isOk = TextureLoader::loadCountAnimationFromBmpFile(std::format("{}/magic/bitmap/{}", rootDirectory, animDesc.animationPath),
                                                                                    animDesc.framesCount, renderer, animation.textures, &transparentColor, &uiError);
                            } else if (animDesc.animationPath.ends_with(".csx")) {
                                isOk = TextureLoader::loadCountAnimationFromCsxFile(std::format("{}/magic/bitmap/{}", rootDirectory, animDesc.animationPath),
                                                                                    animDesc.framesCount, renderer, animation.textures, &uiError);
                            } else {
                                uiError = std::format("Unsupported format! {}", animDesc.animationPath);
                            }

                            layerInfo.animationInfo[animationIndex].width = animation.textures.front()->w;
                            layerInfo.animationInfo[animationIndex].height = animation.textures.front()->h;
                            layerInfo.maxWidth = std::max(animation.textures.front()->w, layerInfo.maxWidth);
                            layerInfo.maxHeight = std::max(animation.textures.front()->h, layerInfo.maxHeight);

                            if (!isOk) break;
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
    if (!animationLayers.empty() && uiError.empty()) {
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
            if (needResetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }

            ImVec2 startPos = ImGui::GetCursorScreenPos();
            uint64_t now = SDL_GetTicks();

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
                    int animPosX = centerW - (animation.currentTexture()->w / 2) + animation.xOffset;
                    int animPosY = centerH - (animation.currentTexture()->h / 2) + animation.yOffset;
                    minX = std::min(animPosX, minX);
                    minY = std::min(animPosY, minY);
                }
            }

            for (auto& layer : animationLayers) {
                for (auto& animation : layer) {
                    assert(!animation.textures.empty());

                    animation.update(now);
                    int animPosX = centerW - (animation.currentTexture()->w / 2);
                    int animPosY = centerH - (animation.currentTexture()->h / 2);
                    const ImVec2 animationPos{startPos.x + animPosX + animation.xOffset - minX, startPos.y + animPosY + animation.yOffset - minY};
                    ImGui::SetCursorScreenPos(animationPos);

                    SDL_SetTextureBlendMode(animation.currentTexture().get(), animation.blendMode());
                    ImVec4 tintColor{1, 1, 1, animation.alpha() / 255.0f};
                    ImGui::ImageWithBg((ImTextureID)animation.currentTexture().get(),
                                       ImVec2(animation.currentTexture()->w, animation.currentTexture()->h),
                                       ImVec2(0, 0), ImVec2(1, 1), bgColor, tintColor);
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
        }
        ImGui::EndGroup();
    } else if (selectedIndex >= 0 && !uiError.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", uiError.c_str());
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        animationLayers.clear();
        layerInfos.clear();
        uiError.clear();
        textFilter.Clear();
    }
}
