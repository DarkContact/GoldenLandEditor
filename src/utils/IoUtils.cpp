#include "IoUtils.h"

int32_t IoUtils::readInt32(std::span<const uint8_t> fileData, size_t& offset) {
    int32_t result = *reinterpret_cast<const int32_t*>(&fileData[offset]);
    offset += sizeof(int32_t);
    return result;
}

float IoUtils::readFloat(std::span<const uint8_t> fileData, size_t& offset) {
    float result = *reinterpret_cast<const float*>(&fileData[offset]);
    offset += sizeof(float);
    return result;
}
