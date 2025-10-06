#include "CSX_Parser.h"

#include "SDL3/SDL_surface.h"

#include <algorithm>
#include <cassert>
#include <array>

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

std::span<const uint32_t> reinterpret_as_u32(std::span<const uint8_t> bytes) {
    assert(reinterpret_cast<uintptr_t>(bytes.data()) % alignof(uint32_t) == 0);
    assert(bytes.size_bytes() % sizeof(uint32_t) == 0);
    return std::span<const uint32_t>(reinterpret_cast<const uint32_t*>(bytes.data()), bytes.size_bytes() / sizeof(uint32_t));
}

SDL_Surface* CSX_Parser::parse(bool isBackgroundTransparent, std::string* error) {
    Tracy_ZoneScopedN("CSX_Parser::parse");

    uint32_t colorCount = readUInt32(); // Читаем количество цветов в палитре
    assert(colorCount > 0);

    ColorData fillColor;
    fillColor.u32 = readUInt32();

    // Читаем палитру цветов
    int fillColorIndex = -1;
    std::array<SDL_Color, 256> SDL_colors = {};
    for (int i = 0; i < colorCount; i++) {
        ColorData color;
        color.u32 = readUInt32();
        if (fillColor.u32 == color.u32) {
            fillColorIndex = i;
        }

        SDL_colors[i].r = color.r;
        SDL_colors[i].g = color.g;
        SDL_colors[i].b = color.b;
        SDL_colors[i].a = 255;
    }
    fillColor.a = 255;

    // Читаем размеры изображения
    uint32_t width = readUInt32();
    uint32_t height = readUInt32();

    // read relative line offsets
    uint32_t lineOffsetSize = (height + 1) * sizeof(uint32_t);
    std::span<const uint32_t> lineOffsets = reinterpret_as_u32(m_buffer.subspan(m_offset, lineOffsetSize));
    m_offset += lineOffsetSize;

    // Читаем данные пикселей
    std::span<const uint8_t> bytes = m_buffer.subspan(m_offset, lineOffsets[height]);

    // Добавляем заполняющий цвет в палитру
    if (fillColorIndex == -1) {
        // Если есть свободное место в палитре
        if (colorCount < 256) {
            SDL_colors[colorCount].r = fillColor.r;
            SDL_colors[colorCount].g = fillColor.g;
            SDL_colors[colorCount].b = fillColor.b;
            fillColorIndex = colorCount;
            ++colorCount;
        } else {
            // Если места нет - анализируем изображение. Если цвет в палитре не используется перезапишем его
            std::array<size_t, 256> frequency = {};
            for (uint8_t b : bytes) {
                ++frequency[b];
            }

            int firstUnusedIndex = -1;
            for (int i = 0; i < 256; ++i) {
                if (frequency[i] == 0) {
                    firstUnusedIndex = i;
                    break;
                }
            }

            if (firstUnusedIndex == -1) {
                // NOTE: Для такого случая можно использовать SDL_PIXELFORMAT_BGRA32
                if (error)
                    *error = "Can't use fillColor value, because pallete is full";
                return nullptr;
            }

            SDL_colors[firstUnusedIndex].r = fillColor.r;
            SDL_colors[firstUnusedIndex].g = fillColor.g;
            SDL_colors[firstUnusedIndex].b = fillColor.b;
            fillColorIndex = firstUnusedIndex;
        }
    }

    // Создаём SDL_Surface
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
    if (!surface) {
        if (error)
            *error = std::string(SDL_GetError());
        return nullptr;
    }
    // Заливка цветом
    SDL_FillSurfaceRect(surface, NULL, fillColorIndex);

    // Создаём и прикрепляем палитру
    SDL_Palette* pallete = SDL_CreatePalette(colorCount);
    SDL_SetPaletteColors(pallete, SDL_colors.data(), 0, colorCount);
    SDL_SetSurfacePalette(surface, pallete);
    SDL_DestroyPalette(pallete);

    if (isBackgroundTransparent) {
        SDL_SetSurfaceColorKey(surface, true, fillColorIndex);
    }

    // Декодируем изображение
    {
        Tracy_ZoneScopedN("decodeLines");
        std::span<uint8_t> pixels((uint8_t*)surface->pixels, surface->pitch * height);
        #pragma omp parallel for
        for (int y = 0; y < height; y++) {
            size_t byteIndex = lineOffsets[y];
            size_t pixelIndex = y * surface->pitch;
            size_t byteCount = lineOffsets[y + 1] - lineOffsets[y];
            decodeLine(bytes, byteIndex, pixels, pixelIndex, width, byteCount);
        }
    }

    return surface;
}

void CSX_Parser::decodeLine(std::span<const uint8_t> bytes, size_t byteIndex, std::span<uint8_t> pixels, size_t pixelIndex, size_t widthLeft, size_t byteCount) {
    while (widthLeft > 0 && byteCount > 0) {
        uint8_t x = bytes[byteIndex];
        byteIndex++;
        byteCount--;

        switch (x) {
            case 107: { // [0x6B] Экранирование. Следующий байт - обычный цвет (для интерпретации 0x69, 0x6A, 0x6C как цвет, а не как команды)
                pixels[pixelIndex] = bytes[byteIndex];
                byteIndex++;
                byteCount--;
                pixelIndex++;
                widthLeft--;
                break;
            }
            case 105: { // [0x69] Прозрачный пиксель
                pixelIndex++;
                widthLeft--;
                break;
            }
            case 106: { // [0x6A] Заполненный цвет
                int runLength = std::min(widthLeft, (size_t)bytes[byteIndex + 1]);
                uint8_t colorIndex = bytes[byteIndex];
                std::fill_n(pixels.begin() + pixelIndex, runLength, colorIndex);
                byteCount -= 2;
                byteIndex += 2;
                pixelIndex += runLength;
                widthLeft -= runLength;
                break;
            }
            case 108: { // [0x6C] Заполнение прозрачным
                int runLength = std::min((size_t)bytes[byteIndex], widthLeft);
                byteIndex++;
                byteCount--;
                pixelIndex += runLength;
                widthLeft -= runLength;
                break;
            }
            default: // Обычный цвет
                pixels[pixelIndex] = x;
                pixelIndex++;
                widthLeft--;
                break;
        }
    }
}
