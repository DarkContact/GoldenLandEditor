#pragma once
#include "stb_image.h"

typedef void* (*StbiMemAllocFunc)(size_t size);
typedef void* (*StbiMemReallocFunc)(void* oldPtr, size_t size);
typedef void  (*StbiMemFreeFunc)(void* ptr);

void stbi_set_allocator_functions(StbiMemAllocFunc allocFunc,
                                  StbiMemReallocFunc reallocFunc,
                                  StbiMemFreeFunc freeFunc);
