#include "PadViewer.h"

#include <algorithm>
#include <cassert>
#include <format>

#include "graphics/TextureLoader.h"
#include "graphics/Texture.h"

#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"
#include "utils/IoUtils.h"

PadViewer::PadViewer() {}

void PadViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& padFiles)
{
    Tracy_ZoneScoped;
    using namespace IoUtils;

    if (showWindow && !padFiles.empty()) {
        m_onceWhenClose = false;
        bool needResetScroll = false;

        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize, ImGuiCond_FirstUseEver);
        ImGui::Begin("PAD Viewer", &showWindow);

        // Left
        {
            ImGui::BeginChild("left pane", ImVec2(400, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            m_textFilter.Draw();
            ImGui::Separator();
                ImGui::BeginChild("file list");
                for (int i = 0; i < static_cast<int>(padFiles.size()); ++i)
                {
                    std::string_view selectedPadFile = padFiles[i];

                    if (m_textFilter.PassFilter(selectedPadFile.data())
                        && ImGui::Selectable(selectedPadFile.data(), m_selectedIndex == i))
                    {
                        m_selectedIndex = i;

                        m_animationTextures.clear();
                        m_padData = PAD_Parser::parse(std::format("{}/{}", rootDirectory, selectedPadFile), &m_error);

                        std::string_view padDir = StringUtils::parentPath(selectedPadFile);
                        for (const auto& animation : m_padData->animations) {
                            auto animationName = animationTypeMaskToString(animation.type);

                            auto animationFilename = std::format("{}/{}/animation/{}.csx", rootDirectory, padDir, animationName);
                            auto shadowFilename = std::format("{}/{}/shadows/{}.csx", rootDirectory, padDir, animationName);

                            Texture animationTexture;
                            std::string animationError;
                            bool isAnimationOk = TextureLoader::loadTextureFromCsxFile(animationFilename, renderer, animationTexture, &animationError);
                            if (!isAnimationOk) {
                                LogFmt("AnimationError: {}", animationError);
                            }

                            Texture shadowTexture;
                            std::string shadowError;
                            bool isShadowOk = TextureLoader::loadTextureFromCsxFile(shadowFilename, renderer, shadowTexture, &shadowError);
                            if (!isShadowOk) {
                                LogFmt("ShadowError: {}", shadowError);
                            }

                            m_animationTextures.insert( {animation.type, std::pair<Texture, Texture>{std::move(animationTexture), std::move(shadowTexture)}} );
                        }

                        needResetScroll = true;
                        m_selectedAnimationIndex = 0;
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        if (!padFiles.empty() && m_padData) {
            ImGui::BeginGroup();

                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() ), 0, ImGuiWindowFlags_HorizontalScrollbar);
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                // ImGui::Text("Size: %zu, Masks: %u", m_padData->animationData.size(), m_padData->animationMasks);
                // for (auto animation : m_padData->animationTypes) {
                //     ImGui::Text("%s", animationTypeMaskToString(animation).data());
                // }

                // for (size_t offset = 0; offset < m_padData->animationData.size();) {
                //     size_t startOffset = offset;
                //     uint32_t value = readUInt32(m_padData->animationData, offset);

                //     offset = startOffset;
                //     uint16_t value16L = readUInt16(m_padData->animationData, offset);
                //     uint16_t value16R = readUInt16(m_padData->animationData, offset);

                //     bool isHighlight = std::ranges::any_of(m_padData->animationTypes, [value](uint32_t x) { return x == value; });

                //     const ImGuiStyle& style = ImGui::GetStyle();
                //     ImVec4 textColor = isHighlight ? ImVec4(1.0f, 0.95f, 0.0f, 1.0f) : style.Colors[ImGuiCol_Text];
                //     if (value > std::numeric_limits<uint16_t>::max()) {
                //         ImGui::TextColored(textColor, "[i:%zu] %u, %u", (offset / 4) - 1, value16L, value16R);
                //     } else {
                //         ImGui::TextColored(textColor, "[i:%zu] %u", (offset / 4) - 1, value);
                //     }
                // }

                // ImGui::Separator();

                // ImGui::Text("%d", m_padData->endSize);
                // for (size_t offset = 0; offset < m_padData->endData.size();) {
                //     uint32_t value = readUInt32(m_padData->endData, offset);
                //     ImGui::Text("[i:%zu] %u", (offset / 4) - 1, value);
                // }

                PAD_AnimationTypeMask currentAnimationType = m_padData->animations[m_selectedAnimationIndex].type;

                const auto& [anim, shadow] = m_animationTextures[currentAnimationType];
                ImGui::ImageWithBg((ImTextureID)anim.get(), ImVec2(anim->w, anim->h), ImVec2(0, 0), ImVec2(1, 1), m_bgColor);
                ImGui::ImageWithBg((ImTextureID)shadow.get(), ImVec2(shadow->w, shadow->h), ImVec2(0, 0), ImVec2(1, 1), m_bgColor);

                ImGui::EndChild();

                assert(!m_padData->animations.empty());
                std::string_view currentAnimationName = animationTypeMaskToString(currentAnimationType);

                if (ImGui::BeginCombo("##Animations", currentAnimationName.data(), ImGuiComboFlags_WidthFitPreview)) {
                    for (int i = 0; i < m_padData->animations.size(); ++i) {
                        bool isSelected = (i == m_selectedAnimationIndex);

                        std::string_view animationName =  animationTypeMaskToString(m_padData->animations[i].type);
                        if (ImGui::Selectable(animationName.data(), isSelected)) {
                            m_selectedAnimationIndex = i;
                            // TODO: Логика обработки анимации
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

            ImGui::EndGroup();
        } else if (m_selectedIndex >= 0) {
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", m_error.c_str());
        }

        ImGui::End();
    }

    // Очистка
    if (!showWindow && !m_onceWhenClose) {
        m_selectedIndex = -1;
        m_onceWhenClose = true;
        m_textFilter.Clear();
        m_error.clear();
        m_padData = {};
        m_animationTextures.clear();
        m_selectedAnimationIndex = -1;
    }
}

bool PadViewer::isAnimating() const {
    return true;
}
