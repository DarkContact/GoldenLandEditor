#include "IoUtils.h"

#include <cassert>

namespace IoUtils
{

std::string_view readString(std::span<const uint8_t> fileData, int stringSize, size_t& offset)
{
    std::string_view sv(reinterpret_cast<const char*>(&fileData[offset]), stringSize);
    offset += stringSize;
    assert(offset <= fileData.size());
    return sv;
}

std::string_view readStringWithSize(std::span<const uint8_t> fileData, size_t& offset)
{
    assert(offset + 4 <= fileData.size());

    int32_t length = readInt32(fileData, offset);
    assert(length >= 0);

    assert(offset + length <= fileData.size());
    if (length == 0) { return {}; }

    std::string_view sv(reinterpret_cast<const char*>(&fileData[offset]), length);
    offset += length;
    return sv;
}

uint16_t readUInt16(std::span<const uint8_t> fileData, size_t& offset)
{
    uint16_t result = *reinterpret_cast<const uint16_t*>(&fileData[offset]);
    offset += sizeof(uint16_t);
    return result;
}

uint32_t readUInt32(std::span<const uint8_t> fileData, size_t& offset)
{
    uint32_t result = *reinterpret_cast<const uint32_t*>(&fileData[offset]);
    offset += sizeof(uint32_t);
    return result;
}

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

void writeStringWithSize(std::vector<uint8_t>& buffer, std::string_view value) {
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
