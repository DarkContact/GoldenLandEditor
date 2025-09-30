#include "CSX_Parser.h"

#include "utils/TracyProfiler.h"

SDL_Surface* CSX_Parser::parse(bool isBackgroundTransparent) {
    Tracy_ZoneScopedN("CSX_Parser::parse");
    // Читаем количество цветов в палитре
    int colorCount = readInt();
    SDL_Color fillColor = readBGRA();
    if (isBackgroundTransparent) {
        fillColor.a = 0;
    }

    // Читаем палитру цветов
    std::vector<SDL_Color> colors;
    for (int i = 0; i < colorCount; i++) {
        colors.push_back(readBGRA());
    }

    // Читаем размеры изображения
    int width = readInt();
    int height = readInt();

    // Читаем индексы строк пикселей
    std::vector<int> byteLineIndices(height + 1);
    for (int i = 0; i < height + 1; i++) {
        byteLineIndices[i] = readInt();
    }

    // Читаем данные пикселей
    std::vector<uint8_t> bytes(byteLineIndices[height]);
    for (int i = 0; i < byteLineIndices[height]; i++) {
        bytes[i] = readByte();
    }

    // Декодируем изображение
    std::vector<int> pixelIndices(width * height, -1);
    for (int y = 0; y < height; y++) {
        decodeLine(bytes, byteLineIndices[y], pixelIndices, y * width, width, byteLineIndices[y + 1] - byteLineIndices[y]);
    }

    // Создаём SDL_Surface
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return nullptr;

    uint8_t* pixels = (uint8_t*)surface->pixels;
    int pitch = surface->pitch; // количество байт в строке

    // Заполняем surface пикселями
    for (int y = 0; y < height; y++) {
        uint8_t* row = pixels + y * pitch;
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            SDL_Color c;
            if (pixelIndices[idx] >= 0) {
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

    return surface;
}

int32_t CSX_Parser::readInt() {
    if (offset + 4 > dataSize) return 0; // Защита от выхода за границы
    int32_t val = *(int32_t*)(data + offset);
    offset += 4;
    return val;
}

uint8_t CSX_Parser::readByte() {
    if (offset + 1 > dataSize) return 0;
    return data[offset++];
}

SDL_Color CSX_Parser::readBGRA() {
    uint8_t b = readByte();
    uint8_t g = readByte();
    uint8_t r = readByte();
    uint8_t a = readByte();
    // alpha инвертирован
    a = 255 - a;
    return SDL_Color{r, g, b, a};
}

void CSX_Parser::decodeLine(const std::vector<uint8_t>& bytes, size_t byteIndex, std::vector<int>& pixels, size_t pixelIndex, int widthLeft, size_t byteCount) {
    Tracy_ZoneScopedN("CSX_Parser::decodeLine");
    size_t startPixelIndex = pixelIndex;
    while (widthLeft > 0 && byteCount > 0) {
        uint8_t x = bytes[byteIndex];
        byteIndex++;
        byteCount--;

        switch (x) {
            case 107: // WTF-case
                pixels[pixelIndex] = bytes[byteIndex];
                if (pixelIndex != startPixelIndex)
                    pixels[pixelIndex - 1] = bytes[byteIndex];
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
                for (int i = 0; i < runLength; i++) {
                    pixels[pixelIndex + i] = colorIndex;
                }
                if (pixelIndex != startPixelIndex)
                    pixels[pixelIndex - 1] = colorIndex;
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
