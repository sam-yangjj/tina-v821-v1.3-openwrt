

#ifndef __TUYA_MUDEX_INTFLOAT_UTIL_H__
#define __TUYA_MUDEX_INTFLOAT_UTIL_H__

#include <stdint.h>

union av_intfloat32 {
    uint32_t i;
    float    f;
};

union av_intfloat64 {
    uint64_t i;
    double   f;
};

/**
 * Reinterpret a 32-bit integer as a float.
 */
static inline float av_int2float(uint32_t i)
{
    union av_intfloat32 v;
    v.i = i;
    return v.f;
}

/**
 * Reinterpret a float as a 32-bit integer.
 */
static inline uint32_t av_float2int(float f)
{
    union av_intfloat32 v;
    v.f = f;
    return v.i;
}

/**
 * Reinterpret a 64-bit integer as a double.
 */
static inline double av_int2double(uint64_t i)
{
    union av_intfloat64 v;
    v.i = i;
    return v.f;
}

/**
 * Reinterpret a double as a 64-bit integer.
 */
static inline uint64_t av_double2int(double f)
{
    union av_intfloat64 v;
    v.f = f;
    return v.i;
}

#endif /* AVUTIL_INTFLOAT_H */
