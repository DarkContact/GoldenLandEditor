#include <cstdint>
#include <string>
#include <span>

struct SDL_Surface;
struct SDL_Palette;

struct CsxMetaInfo {
    static constexpr uint16_t kMaxColors = 256;

    uint32_t colorCount = 0;
    int16_t fillColorIndex = -1;
    uint32_t width = 0;
    uint32_t height = 0;

    std::span<const uint32_t> lineOffsets;
    std::span<const uint8_t> bytes;

    SDL_Palette* pallete = nullptr;
};

class CSX_Parser {
public:
    CSX_Parser(std::span<uint8_t> buffer);
    ~CSX_Parser();

    SDL_Surface* parse(bool isBackgroundTransparent = true, std::string* error = nullptr);

    bool preParse(std::string* error = nullptr);
    bool parseLinesToSurface(SDL_Surface* inOutSurface, bool needFillColor, int lineIndexStart, int lineCount, bool isBackgroundTransparent = true, std::string* error = nullptr);

    const CsxMetaInfo& metaInfo() const;

private:
    void decodeLine(std::span<const uint8_t> bytes, size_t byteIndex,
                    std::span<uint8_t> pixels, size_t pixelIndex,
                    size_t byteCount);

    std::span<uint8_t> m_buffer;
    CsxMetaInfo m_metaInfo;
};
