#pragma once
#include <string_view>
#include <cstdint>
#include <vector>
#include <span>

namespace IoUtils
{

int16_t readInt16(std::span<const uint8_t> fileData, size_t& offset);
int32_t readInt32(std::span<const uint8_t> fileData, size_t& offset);
float readFloat(std::span<const uint8_t> fileData, size_t& offset);

void writeString(std::vector<uint8_t>& buffer, std::string_view value);
void writeSizeAndString(std::vector<uint8_t>& buffer, std::string_view value);
void writeUInt16(std::vector<uint8_t>& buffer, uint16_t value);
void writeUInt32(std::vector<uint8_t>& buffer, uint32_t value);
void writeInt16(std::vector<uint8_t>& buffer, int16_t value);
void writeInt32(std::vector<uint8_t>& buffer, int32_t value);
void writeFloat(std::vector<uint8_t>& buffer, float value);

}
