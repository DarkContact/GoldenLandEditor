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

struct LayerInfo {
    int count = -1;
    int width = -1;
    int height = -1;
};

struct MagicAnimation : public BaseAnimation {
    int xOffset;
    int yOffset;
    bool isBlendModeAdd;
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

static std::string mdfInfoString(const MDF_Data& data) {
    std::string result = std::format("endTimeMs: {}\n\n", data.endTimeMs);
    for (int l = 0; l < data.layers.size(); ++l) {
        const auto& layer = data.layers[l];
        result += std::format("LAYER {}\n", l + 1);
        for (int a = 0; a < layer.animations.size(); ++a) {
            const auto& animation = layer.animations[a];
            result += std::format("ANIMATION {}\n", a + 1);
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
    static std::string mdfDataInfo;
    static std::string uiError;
    static ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    static int activeButtonIndex = 0;
    static bool showInfo = false;
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

                auto mdfDataOpt = MDF_Parser::parse(std::format("{}/{}", rootDirectory, mdfFiles[i]), &uiError);
                if (mdfDataOpt) {
                    auto& mdfData = *mdfDataOpt;
                    mdfDataInfo = mdfInfoString(mdfData);

                    animationLayers.clear();
                    animationLayers.resize(mdfData.layers.size());

                    int layerIndex = 0;
                    for (const auto& layerDesc : mdfData.layers) {
                        auto& animationLayer = animationLayers[layerIndex];
                        animationLayer.resize(layerDesc.animations.size());

                        int animationIndex = 0;
                        for (const auto& animDesc : layerDesc.animations) {
                            auto& animation = animationLayer[animationIndex];
                            animation.delayMs = (animDesc.endTimeMs - animDesc.startTimeMs) / animDesc.framesCount;
                            animation.xOffset = animDesc.xOffset;
                            animation.yOffset = animDesc.yOffset;
                            animation.isBlendModeAdd = animDesc.params.front().flags == 128;

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

                            if (!isOk) break;
                            ++animationIndex;
                        }
                        ++layerIndex;
                    }
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

            ImVec2 originalSpacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(originalSpacing.x, 0)); // убрать вертикальный отступ

            std::array<LayerInfo, 5> layerInfos;
            ImVec2 startPos = ImGui::GetCursorScreenPos();
            uint64_t now = SDL_GetTicks();

            int maxTextureW = 0;
            int maxTextureXOffset = 0;
            int layerIndex = 0;
            for (auto& layer : animationLayers) {
                auto& layerInfo = layerInfos[layerIndex];
                layerInfo.count = layer.size();

                for (auto& animation : layer) {
                    if (animation.textures.empty()) continue;

                    animation.update(now);
                    ImGui::SetCursorScreenPos({startPos.x /*+ animation.xOffset*/, startPos.y /*+ animation.yOffset*/});

                    SDL_SetTextureBlendMode(animation.currentTexture().get(), animation.isBlendModeAdd ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
                    ImGui::ImageWithBg((ImTextureID)animation.currentTexture().get(),
                                       ImVec2(animation.currentTexture()->w, animation.currentTexture()->h),
                                       ImVec2(0, 0), ImVec2(1, 1), bgColor);

                    maxTextureW = std::max(animation.currentTexture()->w, maxTextureW);
                    //maxTextureXOffset = std::max(animation.xOffset, maxTextureXOffset);

                    layerInfo.width = std::max(animation.textures.front()->w, layerInfo.width);
                    layerInfo.height = std::max(animation.textures.front()->h, layerInfo.height);
                }
                ++layerIndex;
            }

            ImGui::PopStyleVar();

            if (showInfo)
            {
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
                                                   layerInfos[i].count, layerInfos[i].width, layerInfos[i].height);
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
        }
        ImGui::EndGroup();
    } else if (selectedIndex >= 0 && !uiError.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", uiError.c_str());
    }

    ImGui::End();

    if (!showWindow) {
        selectedIndex = -1;
        animationLayers.clear();
        uiError.clear();
        textFilter.Clear();
    }
}
