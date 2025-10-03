#include "CSX_Parser.h"

#include <array>
#include "utils/TracyProfiler.h"

CSX_Parser::CSX_Parser(std::span<uint8_t> buffer) :
    m_buffer(buffer)
{

}

SDL_Surface* CSX_Parser::parse(bool isBackgroundTransparent) {
    Tracy_ZoneScopedN("CSX_Parser::parse");
    // Читаем количество цветов в палитре
    int colorCount = readInt();
    SDL_Color fillColor = readBGRA();
    if (isBackgroundTransparent) {
        fillColor.a = 0;
    }

    // Читаем палитру цветов
    std::array<SDL_Color, 256> colors = {};
    for (int i = 0; i < colorCount; i++) {
        colors[i] = readBGRA();
    }

    // Читаем размеры изображения
    int width = readInt();
    int height = readInt();

    // read relative line offsets
    std::vector<int> lineOffsets(height + 1);
    for (int i = 0; i < height + 1; i++) {
        lineOffsets[i] = readInt();
    }

    // Читаем данные пикселей
    std::span<uint8_t> bytes = m_buffer.subspan(m_offset, lineOffsets[height]);

    // Декодируем изображение
    std::vector<uint16_t> pixelIndices(width * height, 0xffff);
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
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (!surface) return nullptr;
    }

    uint8_t* pixels = (uint8_t*)surface->pixels;
    int pitch = surface->pitch; // количество байт в строке

    {
        Tracy_ZoneScopedN("fillSurface");
        // Заполняем surface пикселями
        for (int y = 0; y < height; y++) {
            uint8_t* row = pixels + y * pitch;
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                SDL_Color c;
                if (pixelIndices[idx] != 0xffff) {
                    c = colors[pixelIndices[idx]];
                } else {
                    c = fillColor;
                }
                row[x * 4 + 0] = c.r;
                row[x * 4 + 1] = c.g;
                row[x * 4 + 2] = c.b;
                row[x * 4 + 3] = c.a;
            }
        }
    }

    return surface;
}

void CSX_Parser::decodeLine(std::span<uint8_t> bytes, size_t byteIndex, std::vector<uint16_t>& pixels, size_t pixelIndex, int widthLeft, size_t byteCount) {
    while (widthLeft > 0 && byteCount > 0) {
        uint8_t x = bytes[byteIndex];
        byteIndex++;
        byteCount--;

        switch (x) {
            case 107: // WTF-case
                pixels[pixelIndex] = bytes[byteIndex];
                byteIndex++;
                byteCount--;
                pixelIndex++;
                widthLeft--;
                break;
            case 105: // Прозрачный пиксель
                pixelIndex++;
                widthLeft--;
                break;
            case 106: { // Заполненный цвет
                int runLength = std::min(widthLeft, (int)bytes[byteIndex + 1]);
                uint8_t colorIndex = bytes[byteIndex];
                std::fill_n(pixels.begin() + pixelIndex, runLength, colorIndex);
                byteCount -= 2;
                byteIndex += 2;
                pixelIndex += runLength;
                widthLeft -= runLength;
                break;
            }
            case 108: { // Заполнение прозрачным
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
