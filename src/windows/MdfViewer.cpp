#include "MdfViewer.h"

#include <format>
#include <array>

#include <SDL3/SDL_render.h>

#include "files/BgMdfFile.h"
#include "parsers/MDF_Parser.h"
#include "utils/AnimationCachedLoader.h"
#include "utils/TextureLoader.h"
#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

std::string MdfViewer::mdfParamsString(const MDF_Params& params) {
    return std::format("[p01]: {} [p02]: {} [flags]: {} [nFrame]: {} [p05]: {}\n"
                       "[delay]: {:.2f} [alpha]: {:.2f} [p08]: {:.2f} [p09]: {:.2f} [ms]: {}",
                       params.p01, params.p02, params.flags, params.nFrame, params.p05,
                       params.delayMs, params.alpha, params.p08, params.p09, params.animationTimeMs);
}

std::string MdfViewer::mdfAnimationString(const MDF_Animation& anim) {
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

std::string MdfViewer::mdfInfoString(const MDF_Data& data) {
    assert(data.layers.size() == m_layerInfos.size());

    std::string result = std::format("totalDurationMs: {}\n\n", data.totalDurationMs);
    for (int l = 0; l < data.layers.size(); ++l) {
        const auto& layer = data.layers[l];
        const auto& layerInfo = m_layerInfos[l];
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

MdfViewer::MdfViewer() {

}

void MdfViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& mdfFiles)
{
    Tracy_ZoneScoped;

    if (showWindow && !mdfFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        if (!m_onceWhenOpen) {
            std::string error;
            if (!TextureLoader::loadTextureFromMemory({bg_mdf, bg_mdf_size}, renderer, m_bgTexture, &error))
                LogFmt("Load bgTexture error: {}", error);
            m_onceWhenOpen = true;
        }

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("MDF Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(300, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
                m_textFilter.Draw();
                ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(mdfFiles.size()); ++i)
                {
                    if (m_textFilter.PassFilter(mdfFiles[i].c_str())
                        && ImGui::Selectable(mdfFiles[i].c_str(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        m_animationCurrentTime = 0;
                        m_animationLoader.clear();
                        m_uiError.clear();
                        m_mdfDataInfo.clear();
                        m_animationLayers.clear();
                        m_layerInfos.clear();

                        auto mdfDataOpt = MDF_Parser::parse(std::format("{}/{}", rootDirectory, mdfFiles[i]), &m_uiError);
                        if (mdfDataOpt) {
                            auto& mdfData = *mdfDataOpt;

                            m_animationLayers.resize(mdfData.layers.size());
                            m_layerInfos.resize(mdfData.layers.size());

                            int layerIndex = 0;
                            for (const auto& layerDesc : mdfData.layers) {
                                auto& animationLayer = m_animationLayers[layerIndex];
                                animationLayer.resize(layerDesc.animations.size());

                                auto& layerInfo = m_layerInfos[layerIndex];
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
                                    auto textures = m_animationLoader.loadAnimationCount(animationPath,
                                                                                       animDesc.framesCount,
                                                                                       renderer,
                                                                                       (animDesc.maskAnimationPath.empty() ? &transparentColor : nullptr),
                                                                                       &m_uiError);
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

                            m_mdfDataInfo = mdfInfoString(mdfData);
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
        if (!m_animationLayers.empty() && m_uiError.empty()) {
            ImGui::BeginGroup();
            {
                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 3), 0, ImGuiWindowFlags_HorizontalScrollbar);
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                if (m_showBgTexture) {
                    ImVec2 startPos = ImGui::GetCursorScreenPos();
                    ImGui::Image((ImTextureID)m_bgTexture.get(), ImVec2(m_bgTexture->w, m_bgTexture->h));
                    ImGui::SetCursorScreenPos(startPos);
                }

                int maxTextureW = 0;
                int maxTextureH = 0;
                int maxTextureXOffset = 0;
                for (const auto& layerInfo : m_layerInfos) {
                    maxTextureW = std::max(layerInfo.maxWidth, maxTextureW);
                    maxTextureH = std::max(layerInfo.maxHeight, maxTextureH);
                }

                int centerW = maxTextureW / 2;
                int centerH = maxTextureH / 2;

                // Коррекция отображения чтобы те текстуры которые ушли за {0,0} можно было показать полностью
                int minX = 0;
                int minY = 0;
                for (auto& layer : m_animationLayers) {
                    for (auto& animation : layer) {
                        int animPosX = centerW - (animation.width() / 2) + animation.xOffset;
                        int animPosY = centerH - (animation.height() / 2) + animation.yOffset;
                        minX = std::min(animPosX, minX);
                        minY = std::min(animPosY, minY);
                    }
                }

                ImVec2 startPos = ImGui::GetCursorScreenPos();
                for (auto& layer : m_animationLayers) {
                    for (auto& animation : layer) {
                        animationMaxTime = animation.getTotalDurationMs();

                        animation.update(m_animationCurrentTime);
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
                                               ImVec2(0, 0), ImVec2(1, 1), m_bgColor, tintColor);
                        }
                        maxTextureXOffset = std::max(animation.xOffset, maxTextureXOffset);
                    }
                }

                if (m_showCenter) {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    drawList->AddCircleFilled({startPos.x + centerW - minX, startPos.y + centerH - minY}, 2.0f, IM_COL32(255, 0, 0, 255));
                }

                if (m_showInfo) {
                    ImGui::SetCursorScreenPos({startPos.x + maxTextureW + maxTextureXOffset + 12.0f, startPos.y});

                    ImGuiStyle& style = ImGui::GetStyle();
                    ImGui::PushFont(NULL, style.FontSizeBase - 2.0f);
                    ImGui::Text("%s", m_mdfDataInfo.c_str());
                    ImGui::PopFont();
                }

                ImGui::EndChild();

                ImGui::Checkbox("Play", &m_playAnimation);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(400);

                ImGui::BeginDisabled(m_playAnimation);
                ImGui::SliderInt("Time", &m_animationCurrentTime, 0, animationMaxTime - 1, "%d ms", ImGuiSliderFlags_AlwaysClamp);
                ImGui::EndDisabled();

                // Информация о слоях
                {
                    std::array<char, 256> entries;
                    int offset = 0;
                    for (int i = 0; i < m_animationLayers.size(); ++i) {
                        auto result = std::format_to_n(entries.data() + offset, entries.size() - offset, "L{} = {} ({}x{}), ", i + 1,
                                                       m_layerInfos[i].count, m_layerInfos[i].maxWidth, m_layerInfos[i].maxHeight);
                        offset += result.size;
                    }
                    entries[offset - 2] = '\0';

                    std::array<char, 300> entriesMessage;
                    auto resultFmt = std::format_to_n(entriesMessage.data(), entriesMessage.size() - 1, "Animations count: [{}]", entries.data());
                    entriesMessage[std::min((size_t)resultFmt.size, entriesMessage.size() - 1)] = '\0';
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f) ,"%s", entriesMessage.data());
                }

                if (ImGui::RadioButton("Transparent", m_activeButtonIndex == 0)) {
                    m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
                    m_activeButtonIndex = 0;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Black", m_activeButtonIndex == 1)) {
                    m_bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                    m_activeButtonIndex = 1;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Gray", m_activeButtonIndex == 2)) {
                    m_bgColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                    m_activeButtonIndex = 2;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("White", m_activeButtonIndex == 3)) {
                    m_bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    m_activeButtonIndex = 3;
                }
                ImGui::SameLine();
                ImGui::Checkbox("Info", &m_showInfo);
                ImGui::SameLine();
                ImGui::Checkbox("Center", &m_showCenter);
                ImGui::SameLine();
                ImGui::Checkbox("Background", &m_showBgTexture);
            }
            ImGui::EndGroup();
        } else if (m_selectedIndex >= 0 && !m_uiError.empty()) {
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_uiError.c_str());
        }

        ImGui::End();

        if (m_playAnimation) {
            m_animationCurrentTime += ImGui::GetIO().DeltaTime * 1000;
            if (m_animationCurrentTime >= animationMaxTime) {
                m_animationCurrentTime -= animationMaxTime;

                if (m_animationCurrentTime < 0 || m_animationCurrentTime >= animationMaxTime) {
                    m_animationCurrentTime = 0;
                }
            }
        }
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_animationLoader.clear();
        m_animationCurrentTime = 0;
        m_animationLayers.clear();
        m_layerInfos.clear();
        m_uiError.clear();
        m_textFilter.Clear();
        m_bgTexture = {};
        m_onceWhenOpen = false;
        m_onceWhenClose = true;
    }
}
