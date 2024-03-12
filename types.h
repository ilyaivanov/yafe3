#pragma once

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8 ;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8 ;

typedef float f32;
typedef double f64;

typedef struct MyBitmap
{
    i32 width;
    i32 height;
    i32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;

#define Kilobytes(val) (val * 1024)
#define Megabytes(val) Kilobytes(val * 1024)
#define Gigabytes(val) Megabytes(val * 1024)

#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define Assert(cond) if (!(cond)) { *(u32*)0 = 0; }
#define Fail(msg) Assert(0)

#define VirtualAllocateMemory(size) (VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE))
#define VirtualFreeMemory(ptr) (VirtualFree(ptr, 0, MEM_RELEASE))