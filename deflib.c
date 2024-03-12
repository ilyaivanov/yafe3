#pragma once
#include <stdint.h>

int _fltused = 0x9875;

// Increasing Read Bandwidth with SIMD Instructions https://www.computerenhance.com/p/increasing-read-bandwidth-with-simd
#pragma function(memset)
void *memset(void *dest, int c, size_t count)
{
    char *bytes = (char *)dest;
    while (count--)
    {
        *bytes++ = (char)c;
    }
    return dest;
}
