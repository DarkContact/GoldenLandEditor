#pragma once
#include "stb_image.h"

typedef void* (*StbiMemAllocFunc)(size_t size);
typedef void* (*StbiMemReallocFunc)(void* oldPtr, size_t size);
typedef void  (*StbiMemFreeFunc)(void* ptr);

void stbi_set_allocator_functions(StbiMemAllocFunc allocFunc,
                                  StbiMemReallocFunc reallocFunc,
                                  StbiMemFreeFunc freeFunc);

// Работает только с jpeg
// Декодирование (stbi__YCbCr_to_RGB565_row без SIMD)
// desired_channels - нужно только для одинаковой сигнатуры с stbi_load_from_memory, ни на что не влияет
STBIDEF stbi_uc *stbi_load_rgb565_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
