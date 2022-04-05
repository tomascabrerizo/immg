#ifndef TC_TYPES_H
#define TC_TYPES_H

#include <stdint.h>

#define ASSERT(x) do { if(!(x)) { *((int *)0) = 0; } } while(0)

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#ifdef _TC_ENGINE_BUILD
    #define TC_API DLLEXPORT
#else
    #define TC_API DLLIMPORT
#endif

#define array_count(b) (sizeof((b))/sizeof((b)[0]))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef size_t word;

#define KB(n) (n * 1024ULL)
#define MB(n) (n * 1024ULL * 1024ULL)
#define GB(n) (n * 1024ULL * 1024ULL * 1024ULL)

#endif // TC_TYPES_H
