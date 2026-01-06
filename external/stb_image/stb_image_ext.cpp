#include "stb_image_ext.h"

static void* mallocDefault(size_t size)                   { return malloc(size); }
static void* reallocDefault(void* oldPtr, size_t size)    { return realloc(oldPtr, size); }
static void  freeDefault(void* ptr)                       { free(ptr); }

static StbiMemAllocFunc gAllocFunc      = mallocDefault;
static StbiMemReallocFunc gReallocFunc  = reallocDefault;
static StbiMemFreeFunc gFreeFunc        = freeDefault;

void stbi_set_allocator_functions(StbiMemAllocFunc allocFunc, StbiMemReallocFunc reallocFunc, StbiMemFreeFunc freeFunc)
{
    gAllocFunc = allocFunc;
    gReallocFunc = reallocFunc;
    gFreeFunc = freeFunc;
}

// stb_image configuration
#define STBI_MALLOC(sz)           gAllocFunc(sz)
#define STBI_REALLOC(p,newsz)     gReallocFunc(p,newsz)
#define STBI_FREE(p)              gFreeFunc(p)

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP

#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG
#define STBI_MAX_DIMENSIONS 16384

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
