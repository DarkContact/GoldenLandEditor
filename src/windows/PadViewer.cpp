#include "PadViewer.h"

#include <cassert>
#include <format>

#include "graphics/TextureLoader.h"
#include "graphics/Texture.h"

#include "utils/TracyProfiler.h"
#include "utils/StringUtils.h"
#include "utils/DebugLog.h"

PadViewer::PadViewer() {}

void PadViewer::update(bool& showWindow, SDL_Renderer* renderer, std::string_view rootDirectory, const std::vector<std::string>& padFiles)
{
    Tracy_ZoneScoped;

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
                        m_animationDirectionIndex = static_cast<uint32_t>(PAD_AnimationDirection::down);
                    }
                }
                ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right
        if (!padFiles.empty() && m_padData) {
            ImGui::BeginGroup();

                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), 0, ImGuiWindowFlags_HorizontalScrollbar);
                if (needResetScroll) {
                    ImGui::SetScrollX(0.0f);
                    ImGui::SetScrollY(0.0f);
                }

                PAD_Animation& currentAnimation = m_padData->animations[m_selectedAnimationIndex];
                PAD_AnimationTypeMask currentAnimationType = currentAnimation.type; 

                auto& [personTexture, shadowTexture] = m_animationTextures[currentAnimationType];
                m_personAnimation.setAnimation(&personTexture, &shadowTexture, &currentAnimation);
                m_personAnimation.update(m_animationCurrentTime, m_animationDirectionIndex);

                ImVec2 startPersonAnimation = ImGui::GetCursorScreenPos();
                ImGui::ImageWithBg((ImTextureID)personTexture.get(),
                                   ImVec2(currentAnimation.frameWidth, currentAnimation.frameHeight),
                                   m_personAnimation.personUvTopLeft(),
                                   m_personAnimation.personUvBottomRight(),
                                   m_bgColor);

                ImGui::SetCursorScreenPos({startPersonAnimation.x + m_personAnimation.shadowOffset().x,
                                          startPersonAnimation.y + m_personAnimation.shadowOffset().y});

                // TODO: Добавить полупрозрачность тени
                ImGui::ImageWithBg((ImTextureID)shadowTexture.get(),
                                   ImVec2(currentAnimation.shadowFrame.width, currentAnimation.shadowFrame.height),
                                   m_personAnimation.shadowUvTopLeft(),
                                   m_personAnimation.shadowUvBottomRight());

                ImGui::Text("move x: %f, y: %f", currentAnimation.movementX, currentAnimation.movementY);

                ImGui::EndChild();

                // Нижнее меню
                int animationMaxTime = currentAnimation.delay * currentAnimation.framesPerRow;

                ImGui::Checkbox("Play", &m_playAnimation);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(400);

                ImGui::BeginDisabled(m_playAnimation);
                ImGui::SliderInt("Time", &m_animationCurrentTime, 0, animationMaxTime - 1, "%d ms", ImGuiSliderFlags_AlwaysClamp);
                ImGui::EndDisabled();

                assert(!m_padData->animations.empty());
                std::string_view currentAnimationName = animationTypeMaskToString(currentAnimationType);

                bool isAnimationDirectionIndexChange = false;
                uint32_t animationDirectionIndexChange = m_animationDirectionIndex;

                int nextSelectedAnimationIndex = m_selectedAnimationIndex;
                int animsCount = static_cast<int>(m_padData->animations.size());

                ImGui::SetNextItemWidth(80.0f);
                if (ImGui::BeginCombo("##Animations", currentAnimationName.data())) {
                    for (int i = 0; i < m_padData->animations.size(); ++i) {
                        bool isSelected = (i == m_selectedAnimationIndex);

                        std::string_view animationName = animationTypeMaskToString(m_padData->animations[i].type);
                        if (ImGui::Selectable(animationName.data(), isSelected)) {
                            nextSelectedAnimationIndex = i;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();

                if (ImGui::ArrowButton("##PrevAnim", ImGuiDir_Left)) {
                    nextSelectedAnimationIndex = (m_selectedAnimationIndex - 1 + animsCount) % animsCount;
                }
                ImGui::SameLine();

                if (ImGui::ArrowButton("##NextAnim", ImGuiDir_Right)) {
                    nextSelectedAnimationIndex = (m_selectedAnimationIndex + 1) % animsCount;
                }

                // Единая точка обновления индекса и перерасчёта направлений при изменении выбора
                if (nextSelectedAnimationIndex != m_selectedAnimationIndex) {
                    int32_t prevCount = m_padData->animations[m_selectedAnimationIndex].shadowRowCount;
                    int32_t currentCount = m_padData->animations[nextSelectedAnimationIndex].shadowRowCount;
                    m_selectedAnimationIndex = nextSelectedAnimationIndex;

                    // Если несовпало количество строк в анимациях сделаем перерасчёт
                    if (prevCount != currentCount) {
                        isAnimationDirectionIndexChange = true;
                        if (currentCount == PAD_Data::kAnimationGoRowCount) {
                            animationDirectionIndexChange *= 2;
                        } else if (currentCount == PAD_Data::kAnimationRowCount) {
                            animationDirectionIndexChange /= 2;
                        }
                    }
                }

                bool isGoType = PAD_Data::isGoType(currentAnimationType);

                std::string_view currentDirectionName;
                if (isGoType) {
                    currentDirectionName = animationDirectionGoToString(PAD_Data::animationDirectionsGoShadow[m_animationDirectionIndex]);
                } else {
                    currentDirectionName = animationDirectionToString(PAD_Data::animationDirectionsShadow[m_animationDirectionIndex]);
                }

                ImGui::SameLine(0, 30.0f);

                ImGui::SetNextItemWidth(100.0f);
                if (ImGui::BeginCombo("##Directions", currentDirectionName.data())) {
                    for (int i = 0; i < currentAnimation.shadowRowCount; ++i) {
                        bool isSelected = (i == m_animationDirectionIndex);

                        std::string_view directionName;
                        if (isGoType) {
                            directionName = animationDirectionGoToString(PAD_Data::animationDirectionsGoShadow[i]);
                        } else {
                            directionName = animationDirectionToString(PAD_Data::animationDirectionsShadow[i]);
                        }
                        if (ImGui::Selectable(directionName.data(), isSelected)) {
                            m_animationDirectionIndex = i;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();

                int dirSize = currentAnimation.shadowRowCount;
                if (ImGui::Button("CCW")) {
                    if (dirSize > 0) {
                        int nextDirection = static_cast<int>(m_animationDirectionIndex);
                        nextDirection = (nextDirection + 1) % dirSize;
                        m_animationDirectionIndex = static_cast<uint32_t>(nextDirection);
                    }
                }
                ImGui::SameLine();

                if (ImGui::Button("CW")) {
                    if (dirSize > 0) {
                        int nextDirection = static_cast<int>(m_animationDirectionIndex);
                        nextDirection = (nextDirection - 1 + dirSize) % dirSize;
                        m_animationDirectionIndex = static_cast<uint32_t>(nextDirection);
                    }
                }

                if (m_playAnimation) {
                    m_animationCurrentTime += ImGui::GetIO().DeltaTime * 1000;
                    if (m_animationCurrentTime >= animationMaxTime) {
                        m_animationCurrentTime -= animationMaxTime;

                        if (m_animationCurrentTime < 0 || m_animationCurrentTime >= animationMaxTime) {
                            m_animationCurrentTime = 0;
                        }
                    }
                }

                if (isAnimationDirectionIndexChange) {
                    m_animationDirectionIndex = animationDirectionIndexChange;
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
        m_animationDirectionIndex = -1;
        m_animationCurrentTime = 0;
    }
}

bool PadViewer::isAnimating() const {
    return m_playAnimation && m_padData;
}
