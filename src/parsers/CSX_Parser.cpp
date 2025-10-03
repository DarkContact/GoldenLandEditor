#include "CSX_Parser.h"

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

SDL_Surface* CSX_Parser::parse(bool isBackgroundTransparent) {
    Tracy_ZoneScopedN("CSX_Parser::parse");
    uint32_t colorCount = readUInt32(); // Читаем количество цветов в палитре
    ColorData fillColor;
    fillColor.u32 = readUInt32();

    // Читаем палитру цветов
    int transparentIndex = -1;
    std::array<ColorData, 256> colors = {};
    for (int i = 0; i < colorCount; i++) {
        colors[i].u32 = readUInt32();
        if (fillColor.u32 == colors[i].u32) {
            transparentIndex = i;
        }
        colors[i].a = 255;
    }
    if (isBackgroundTransparent && transparentIndex >= 0) {
        colors[transparentIndex].a = 0;
    }

    // Читаем размеры изображения
    uint32_t width = readUInt32();
    uint32_t height = readUInt32();

    // read relative line offsets
    std::vector<uint32_t> lineOffsets(height + 1);
    for (int i = 0; i < height + 1; i++) {
        lineOffsets[i] = readUInt32();
    }

    // Читаем данные пикселей
    std::span<uint8_t> bytes = m_buffer.subspan(m_offset, lineOffsets[height]);

    // Декодируем изображение
    std::vector<uint8_t> pixelIndices(width * height, transparentIndex);
    {
        Tracy_ZoneScopedN("decodeLines");
        for (int y = 0; y < height; y++) {
            decodeLine(bytes, lineOffsets[y], pixelIndices, y * width, width, lineOffsets[y + 1] - lineOffsets[y]);
        }
    }

    // Создаём SDL_Surface
    SDL_Surface* surface;
    {
        Tracy_ZoneScopedN("createSurface");
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_BGRA32);
        if (!surface) return nullptr;
    }

    uint8_t* pixels = (uint8_t*)surface->pixels;
    int pitch = surface->pitch; // количество байт в строке
    {
        Tracy_ZoneScopedN("fillSurface");
        // Заполняем surface пикселями
        for (int y = 0; y < height; y++) {
            uint32_t* row = reinterpret_cast<uint32_t*>(pixels + y * pitch);
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                row[x] = colors[pixelIndices[idx]].u32;
            }
        }
    }

    return surface;
}

void CSX_Parser::decodeLine(std::span<uint8_t> bytes, size_t byteIndex, std::vector<uint8_t>& pixels, size_t pixelIndex, int widthLeft, size_t byteCount) {
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
                int runLength = std::min(widthLeft, (int)bytes[byteIndex + 1]);
                uint8_t colorIndex = bytes[byteIndex];
                std::fill_n(pixels.begin() + pixelIndex, runLength, colorIndex);
                byteCount -= 2;
                byteIndex += 2;
                pixelIndex += runLength;
                widthLeft -= runLength;
                break;
            }
            case 108: { // [0x6C] Заполнение прозрачным
                int runLength = std::min((int)bytes[byteIndex], widthLeft);
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
