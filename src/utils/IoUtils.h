#pragma once
#include <cstdint>
#include <span>

class IoUtils
{
public:
    IoUtils() = delete;

    static int32_t readInt32(std::span<const uint8_t> fileData, size_t& offset);
    static float readFloat(std::span<const uint8_t> fileData, size_t& offset);
};

