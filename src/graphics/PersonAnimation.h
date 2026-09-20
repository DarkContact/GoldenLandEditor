#pragma once
#include <cstdint>

#include "imgui.h"

#include "parsers/PAD_Parser.h"
#include "graphics/Texture.h"

class PersonAnimation {
public:
    void setAnimation(Texture* personTexture, Texture* shadowTexture, PAD_Animation* animation) {
        m_personTexture = personTexture;
        m_shadowTexture = shadowTexture;
        m_animation = animation;

        m_shadowOffset.x = m_animation->anchorX - m_animation->shadowFrame.anchorX;
        m_shadowOffset.y = m_animation->anchorY - m_animation->shadowFrame.anchorY;

        m_currentFrame = 0;
        m_currentTimeMs = 0;

        m_directionIndex = static_cast<uint32_t>(PAD_AnimationDirection::up);
        calculateUvPoints();
    }

    Texture* personTexture() const { return m_personTexture; }
    Texture* shadowTexture() const { return m_shadowTexture; }
    ImVec2 shadowOffset() const { return m_shadowOffset; }

    ImVec2 personUvTopLeft() const { return m_personUvTopLeft; }
    ImVec2 personUvBottomRight() const { return m_personUvBottomRight; }

    ImVec2 shadowUvTopLeft() const { return m_shadowUvTopLeft; }
    ImVec2 shadowUvBottomRight() const { return m_shadowUvBottomRight; }

    // currentTimeMs - валидируется извне, весь duration считается как m_animation->framesPerRow * m_animation->delay
    // directionIndex - валидируется извне, для анимаций типа _go - можно задать 16 направлений, для остальных - 8
    void update(uint64_t currentTimeMs, uint32_t directionIndex) {
        assert(m_animation);

        m_currentTimeMs = currentTimeMs;
        m_currentFrame = m_currentTimeMs / m_animation->delay;

        m_directionIndex = directionIndex;
        calculateUvPoints();
    }

private:
    void calculateUvPoints() {
        bool isGoType = m_animation->type == PAD_AnimationTypeMask::tb_go
                        || m_animation->type == PAD_AnimationTypeMask::rt_go;

        uint32_t rowPerson;
        uint32_t rowShadow;
        if (isGoType) {
            rowPerson = static_cast<uint32_t>(PAD_Data::animationDirectionsGoPerson[m_directionIndex]);
            rowShadow = static_cast<uint32_t>(PAD_Data::animationDirectionsGoShadow[m_directionIndex]);
        } else {
            rowPerson = static_cast<uint32_t>(PAD_Data::animationDirectionsPerson[m_directionIndex]);
            rowShadow = static_cast<uint32_t>(PAD_Data::animationDirectionsShadow[m_directionIndex]);
        }
        bool isPersonMirrorX = (m_directionIndex != rowPerson);

        uint32_t personX = m_animation->frameWidth * m_currentFrame;
        uint32_t personY = m_animation->frameHeight * rowPerson;

        uint32_t shadowX = m_animation->shadowFrame.width * m_currentFrame;
        uint32_t shadowY = m_animation->shadowFrame.height * rowShadow;

        m_personUvTopLeft.x = personX / m_personTexture->get()->w;
        m_personUvTopLeft.y = personY / m_personTexture->get()->h;

        m_personUvBottomRight.x = (personX + m_animation->frameWidth) / m_personTexture->get()->w;
        m_personUvBottomRight.y = (personY + m_animation->frameHeight) / m_personTexture->get()->h;

        if (isPersonMirrorX) {
            std::swap(m_personUvTopLeft.x, m_personUvBottomRight.x);
        }

        m_shadowUvTopLeft.x = shadowX / m_shadowTexture->get()->w;
        m_shadowUvTopLeft.y = shadowY / m_shadowTexture->get()->h;

        m_shadowUvBottomRight.x = (shadowX + m_animation->shadowFrame.width) / m_shadowTexture->get()->w;
        m_shadowUvBottomRight.y = (shadowY + m_animation->shadowFrame.height) / m_shadowTexture->get()->h;
    }

    Texture* m_personTexture;
    Texture* m_shadowTexture;
    PAD_Animation* m_animation = nullptr;
    uint32_t m_directionIndex = static_cast<uint32_t>(PAD_AnimationDirection::up);

    ImVec2 m_shadowOffset;
    ImVec2 m_personUvTopLeft;
    ImVec2 m_personUvBottomRight;
    ImVec2 m_shadowUvTopLeft;
    ImVec2 m_shadowUvBottomRight;

    uint32_t m_currentFrame = 0;
    uint64_t m_currentTimeMs = 0;
};

