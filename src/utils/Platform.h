#pragma once
// https://github.com/bkaradzic/bx/blob/master/include/bx/platform.h

// Usage:
// BX_ARCH_NAME
// BX_COMPILER_NAME

#define BX_STRINGIZE(_x) BX_STRINGIZE_(_x)
#define BX_STRINGIZE_(_x) #_x

// Architecture
#define BX_ARCH_32BIT 0
#define BX_ARCH_64BIT 0

// Compiler
#define BX_COMPILER_CLANG 0
#define BX_COMPILER_GCC   0
#define BX_COMPILER_MSVC  0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
#	undef  BX_COMPILER_CLANG
#	define BX_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(_MSC_VER)
#	undef  BX_COMPILER_MSVC
#	define BX_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  BX_COMPILER_GCC
#	define BX_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "BX_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     \
 || defined(__aarch64__) \
 || defined(_M_ARM)
#	undef  BX_CPU_ARM
#	define BX_CPU_ARM 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    \
 ||   defined(_M_X64)     \
 ||   defined(__i386__)   \
 ||   defined(__x86_64__)
#	undef  BX_CPU_X86
#	define BX_CPU_X86 1
#	define BX_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    \
 || defined(_M_X64)        \
 || defined(__aarch64__)   \
 || defined(__64BIT__)     \
 || defined(__LP64__)
#	undef  BX_ARCH_64BIT
#	define BX_ARCH_64BIT 64
#else
#	undef  BX_ARCH_32BIT
#	define BX_ARCH_32BIT 32
#endif //


#if BX_COMPILER_GCC
#	define BX_COMPILER_NAME "GCC "       \
        BX_STRINGIZE(__GNUC__) "."       \
        BX_STRINGIZE(__GNUC_MINOR__) "." \
        BX_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif BX_COMPILER_CLANG
#	define BX_COMPILER_NAME "Clang "      \
        BX_STRINGIZE(__clang_major__) "." \
        BX_STRINGIZE(__clang_minor__) "." \
        BX_STRINGIZE(__clang_patchlevel__)
#elif BX_COMPILER_MSVC
#	define BX_COMPILER_NAME "MSVC "       \
        BX_STRINGIZE(_MSC_VER)
#endif // BX_COMPILER_

#if BX_ARCH_32BIT
#	define BX_ARCH_NAME "32-bit"
#elif BX_ARCH_64BIT
#	define BX_ARCH_NAME "64-bit"
#endif // BX_ARCH_
