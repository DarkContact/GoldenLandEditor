#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <span>

class CSX_Parser {
public:
    CSX_Parser(std::span<uint8_t> buffer);

    SDL_Surface* parse(bool isBackgroundTransparent = true);

private:
    inline uint8_t readByte() { return m_buffer[m_offset++]; }
    inline int32_t readInt() {
        int32_t val = *(int32_t*)(m_buffer.data() + m_offset);
        m_offset += 4;
        return val;
    }
    inline SDL_Color readBGRA() {
        uint8_t b = readByte();
        uint8_t g = readByte();
        uint8_t r = readByte();
        uint8_t a = readByte();
        a = 255 - a; // alpha инвертирован
        return SDL_Color{r, g, b, a};
    }

    void decodeLine(std::span<uint8_t> bytes, size_t byteIndex,
                    std::vector<uint16_t>& pixels, size_t pixelIndex,
                    int widthLeft, size_t byteCount);

    std::span<uint8_t> m_buffer;
    size_t m_offset = 0;
};
