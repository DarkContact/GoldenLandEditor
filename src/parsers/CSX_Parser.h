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
    inline uint32_t readUInt32() {
        uint32_t val = *(uint32_t*)(m_buffer.data() + m_offset);
        m_offset += 4;
        return val;
    }

    void decodeLine(std::span<uint8_t> bytes, size_t byteIndex,
                    std::vector<uint8_t>& pixels, size_t pixelIndex,
                    int widthLeft, size_t byteCount);

    std::span<uint8_t> m_buffer;
    size_t m_offset = 0;
};
