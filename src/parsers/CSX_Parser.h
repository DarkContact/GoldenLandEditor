#include <cstdint>
#include <string>
#include <span>

struct SDL_Surface;

class CSX_Parser {
public:
    CSX_Parser(std::span<uint8_t> buffer);

    SDL_Surface* parse(bool isBackgroundTransparent = true, std::string* error = nullptr);

private:
    inline uint32_t readUInt32() {
        uint32_t val = *(uint32_t*)(m_buffer.data() + m_offset);
        m_offset += sizeof(uint32_t);
        return val;
    }

    void decodeLine(std::span<const uint8_t> bytes, size_t byteIndex,
                    std::span<uint8_t> pixels, size_t pixelIndex,
                    int widthLeft, size_t byteCount);

    std::span<uint8_t> m_buffer;
    size_t m_offset = 0;
};
