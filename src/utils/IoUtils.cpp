#include "IoUtils.h"

namespace IoUtils
{

int16_t readInt16(std::span<const uint8_t> fileData, size_t& offset)
{
    int16_t result = *reinterpret_cast<const int16_t*>(&fileData[offset]);
    offset += sizeof(int16_t);
    return result;
}

int32_t readInt32(std::span<const uint8_t> fileData, size_t& offset) {
    int32_t result = *reinterpret_cast<const int32_t*>(&fileData[offset]);
    offset += sizeof(int32_t);
    return result;
}

float readFloat(std::span<const uint8_t> fileData, size_t& offset) {
    float result = *reinterpret_cast<const float*>(&fileData[offset]);
    offset += sizeof(float);
    return result;
}

void writeString(std::vector<uint8_t>& buffer, std::string_view value) {
    buffer.insert(buffer.end(), value.begin(), value.end());
}

void writeSizeAndString(std::vector<uint8_t>& buffer, std::string_view value) {
    writeUInt32(buffer, value.size());
    writeString(buffer, value);
}

void writeUInt16(std::vector<uint8_t>& buffer, uint16_t value) {
    uint8_t bytes[2];
    std::memcpy(bytes, &value, sizeof(uint16_t));
    buffer.insert(buffer.end(), bytes, bytes + 2);
}

void writeUInt32(std::vector<uint8_t>& buffer, uint32_t value) {
    uint8_t bytes[4];
    std::memcpy(bytes, &value, sizeof(uint32_t));
    buffer.insert(buffer.end(), bytes, bytes + 4);
}

void writeInt16(std::vector<uint8_t>& buffer, int16_t value)
{
    uint8_t bytes[2];
    std::memcpy(bytes, &value, sizeof(int16_t));
    buffer.insert(buffer.end(), bytes, bytes + 2);
}

void writeInt32(std::vector<uint8_t>& buffer, int32_t value)
{
    uint8_t bytes[4];
    std::memcpy(bytes, &value, sizeof(int32_t));
    buffer.insert(buffer.end(), bytes, bytes + 4);
}

void writeFloat(std::vector<uint8_t>& buffer, float value) {
    uint8_t bytes[4];
    std::memcpy(bytes, &value, sizeof(float));
    buffer.insert(buffer.end(), bytes, bytes + 4);
}



} // IoUtils
