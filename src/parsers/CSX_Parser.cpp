#include "CSX_Parser.h"

#include <algorithm>
#include <cassert>
#include <array>

#include "SDL3/SDL_surface.h"

#include "utils/TracyProfiler.h"

union ColorData {
    uint32_t u32;
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
};

CSX_Parser::CSX_Parser(std::span<uint8_t> buffer) :
    m_buffer(buffer)
{

}

CSX_Parser::~CSX_Parser()
{
    if (m_metaInfo.pallete)
        SDL_DestroyPalette(m_metaInfo.pallete);
}

std::span<const uint32_t> reinterpret_as_u32(std::span<const uint8_t> bytes) {
    assert(reinterpret_cast<uintptr_t>(bytes.data()) % alignof(uint32_t) == 0);
    assert(bytes.size_bytes() % sizeof(uint32_t) == 0);
    return std::span<const uint32_t>(reinterpret_cast<const uint32_t*>(bytes.data()), bytes.size_bytes() / sizeof(uint32_t));
}

SDL_Surface* CSX_Parser::parse(bool isBackgroundTransparent, std::string* error) {
    Tracy_ZoneScopedN("CSX_Parser::parse");

    if (!preParse(error))
        return nullptr;

    // Создаём SDL_Surface
    SDL_Surface* surface = SDL_CreateSurface(m_metaInfo.width, m_metaInfo.height, SDL_PIXELFORMAT_INDEX8);
    if (!surface) {
        if (error)
            *error = SDL_GetError();
        return nullptr;
    }

    const bool needFillColor = (m_metaInfo.fillColorIndex != 0);
    if (!parseLinesToSurface(surface, needFillColor, 0, m_metaInfo.height, isBackgroundTransparent, error)) {
        SDL_DestroySurface(surface);
        return nullptr;
    }

    return surface;
}

bool CSX_Parser::preParse(std::string* error)
{
    assert(!m_metaInfo.pallete);
    assert(m_offset == 0);

    m_metaInfo.colorCount = readUInt32(); // Читаем количество цветов в палитре
    assert(m_metaInfo.colorCount > 0);

    ColorData fillColor;
    fillColor.u32 = readUInt32();

    // Читаем палитру цветов
    std::array<SDL_Color, CsxMetaInfo::kMaxColors> palleteColors;
    for (int i = 0; i < m_metaInfo.colorCount; i++) {
        ColorData color;
        color.u32 = readUInt32();
        if (fillColor.u32 == color.u32) {
            m_metaInfo.fillColorIndex = i;
        }

        palleteColors[i].r = color.r;
        palleteColors[i].g = color.g;
        palleteColors[i].b = color.b;
        palleteColors[i].a = 255;
    }

    // Читаем размеры изображения
    m_metaInfo.width = readUInt32();
    m_metaInfo.height = readUInt32();

    // read relative line offsets
    uint32_t lineOffsetSize = (m_metaInfo.height + 1) * sizeof(uint32_t);
    m_metaInfo.lineOffsets = reinterpret_as_u32(m_buffer.subspan(m_offset, lineOffsetSize));
    m_offset += lineOffsetSize;

    // Читаем данные пикселей
    m_metaInfo.bytes = m_buffer.subspan(m_offset, m_metaInfo.lineOffsets[m_metaInfo.height]);

    // Добавляем заполняющий цвет в палитру
    if (m_metaInfo.fillColorIndex == -1) {
        // Если есть свободное место в палитре
        if (m_metaInfo.colorCount < CsxMetaInfo::kMaxColors) {
            palleteColors[m_metaInfo.colorCount].r = fillColor.r;
            palleteColors[m_metaInfo.colorCount].g = fillColor.g;
            palleteColors[m_metaInfo.colorCount].b = fillColor.b;
            m_metaInfo.fillColorIndex = m_metaInfo.colorCount;
            ++m_metaInfo.colorCount;
        } else {
            // Если места нет - анализируем изображение. Если цвет в палитре не используется перезапишем его
            std::array<size_t, CsxMetaInfo::kMaxColors> frequency = {};
            for (uint8_t b : m_metaInfo.bytes) {
                ++frequency[b];
            }

            int firstUnusedIndex = -1;
            for (int i = 0; i < CsxMetaInfo::kMaxColors; ++i) {
                if (frequency[i] == 0) {
                    firstUnusedIndex = i;
                    break;
                }
            }

            // При проверке всех csx таких случаев не было
            if (firstUnusedIndex == -1) {
                if (error)
                    *error = "Can't use fillColor value, because pallete is full";
                return false;
            }

            palleteColors[firstUnusedIndex].r = fillColor.r;
            palleteColors[firstUnusedIndex].g = fillColor.g;
            palleteColors[firstUnusedIndex].b = fillColor.b;
            m_metaInfo.fillColorIndex = firstUnusedIndex;
        }
    }

    // Создаём палитру
    m_metaInfo.pallete = SDL_CreatePalette(m_metaInfo.colorCount);
    SDL_SetPaletteColors(m_metaInfo.pallete, palleteColors.data(), 0, m_metaInfo.colorCount);

    return true;
}

bool CSX_Parser::parseLinesToSurface(SDL_Surface* inOutSurface, bool needFillColor, int lineIndexStart, int lineCount, bool isBackgroundTransparent, std::string* error)
{
    // Заливка цветом
    if (needFillColor)
        SDL_FillSurfaceRect(inOutSurface, NULL, m_metaInfo.fillColorIndex);

    // Прикрепляем палитру
    SDL_SetSurfacePalette(inOutSurface, m_metaInfo.pallete);

    // Устанавливаем прозрачный цвет
    if (isBackgroundTransparent) {
        SDL_SetSurfaceColorKey(inOutSurface, true, m_metaInfo.fillColorIndex);
    }

    // Декодируем изображение
    std::span<uint8_t> pixels((uint8_t*)inOutSurface->pixels, inOutSurface->pitch * lineCount);
    #pragma omp parallel for
    for (int y = lineIndexStart; y < lineIndexStart + lineCount; y++) {
        size_t byteIndex = m_metaInfo.lineOffsets[y];
        size_t pixelIndex = (y - lineIndexStart) * inOutSurface->pitch;
        size_t byteCount = m_metaInfo.lineOffsets[y + 1] - m_metaInfo.lineOffsets[y];
        decodeLine(m_metaInfo.bytes, byteIndex, pixels, pixelIndex, byteCount);
    }

    return true;
}

void CSX_Parser::decodeLine(std::span<const uint8_t> bytes, size_t byteIndex, std::span<uint8_t> pixels, size_t pixelIndex, size_t byteCount) {
    while (byteCount > 0) {
        uint8_t x = bytes[byteIndex];
        byteIndex++;
        byteCount--;

        switch (x) {
            case 107: { // [0x6B] Экранирование. Следующий байт - обычный цвет (для интерпретации 0x6B, 0x69, 0x6A, 0x6C как цвет, а не как команды)
                pixels[pixelIndex] = bytes[byteIndex];
                byteIndex++;
                byteCount--;
                pixelIndex++;
                break;
            }
            case 105: { // [0x69] Прозрачный пиксель
                pixelIndex++;
                break;
            }
            case 106: { // [0x6A] Заполненный цвет
                auto runLength = bytes[byteIndex + 1];
                uint8_t colorIndex = bytes[byteIndex];
                std::fill_n(pixels.begin() + pixelIndex, runLength, colorIndex);
                byteCount -= 2;
                byteIndex += 2;
                pixelIndex += runLength;
                break;
            }
            case 108: { // [0x6C] Заполнение прозрачным
                auto runLength = bytes[byteIndex];
                byteIndex++;
                byteCount--;
                pixelIndex += runLength;
                break;
            }
            default: // Обычный цвет
                pixels[pixelIndex] = x;
                pixelIndex++;
                break;
        }
    }
}

const CsxMetaInfo& CSX_Parser::metaInfo() const {
    return m_metaInfo;
}
