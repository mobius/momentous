#pragma once

#include <stdint.h>

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif
	
#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

typedef int8_t	s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float f32;
typedef double d32;