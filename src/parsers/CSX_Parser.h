#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>
#include <cstring>

class CSX_Parser {
public:
    CSX_Parser(const uint8_t* buffer, size_t size) : data(buffer), dataSize(size) {}

    SDL_Surface* parse(bool isBackgroundTransparent = true);

private:
    int32_t readInt();

    uint8_t readByte();

    SDL_Color readBGRA();

    void decodeLine(const std::vector<uint8_t>& bytes, size_t byteIndex, std::vector<int>& pixels,
                    size_t pixelIndex, int widthLeft, size_t byteCount);

    const uint8_t* data;
    size_t dataSize;
    size_t offset = 0;
};
