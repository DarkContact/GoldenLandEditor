#pragma once
#include <string_view>
#include <cstdint>
#include <vector>
#include <span>

namespace IoUtils
{
    std::string_view readString(std::span<const uint8_t> fileData, int stringSize, size_t& offset);
    std::string_view readStringWithSize(std::span<const uint8_t> fileData, size_t& offset);
    uint16_t readUInt16(std::span<const uint8_t> fileData, size_t& offset);
    uint32_t readUInt32(std::span<const uint8_t> fileData, size_t& offset);
    int16_t readInt16(std::span<const uint8_t> fileData, size_t& offset);
    int32_t readInt32(std::span<const uint8_t> fileData, size_t& offset);
    float readFloat(std::span<const uint8_t> fileData, size_t& offset);

    void writeString(std::vector<uint8_t>& buffer, std::string_view value);
    void writeStringWithSize(std::vector<uint8_t>& buffer, std::string_view value);
    void writeUInt16(std::vector<uint8_t>& buffer, uint16_t value);
    void writeUInt32(std::vector<uint8_t>& buffer, uint32_t value);
    void writeInt16(std::vector<uint8_t>& buffer, int16_t value);
    void writeInt32(std::vector<uint8_t>& buffer, int32_t value);
    void writeFloat(std::vector<uint8_t>& buffer, float value);
}
