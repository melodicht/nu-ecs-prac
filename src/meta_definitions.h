#pragma once

// SKL_SLOW
// 0 - No slow code allowed
// 1 - Slow code allowed

// SKL_INTERNAL
// 0 - For release
// 1 - Not for release

// Project Fixed Width Numerical Types
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef s32 b32;

// Project Keywords
#define global_variable static
#define local static
#define local_persist static

// Misc

// Assumes that the Expression evaluates to an array.
#define ArrayCount(Expression) (sizeof(Expression)/sizeof((Expression)[0]))

#if SKL_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0; }
#else
#define Assert(Expression)
#endif
