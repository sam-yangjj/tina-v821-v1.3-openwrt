#ifndef __TUYA_MUDEX_INTMATH_UTIL_H__
#define __TUYA_MUDEX_INTMATH_UTIL_H__

#include <stdint.h>


extern const uint8_t ff_log2_tab[256];



/* Pull in unguarded fallback defines at the end of this file. */
//#include "common.h"

#ifndef av_log2
 int av_log2(unsigned v);
#endif

#ifndef av_log2_16bit
 int av_log2_16bit(unsigned v);
#endif

#ifndef ff_ctz
#define ff_ctz ff_ctz_c
static inline  int ff_ctz_c(int v)
{
    static const uint8_t debruijn_ctz32[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    return debruijn_ctz32[(uint32_t)((v & -v) * 0x077CB531U) >> 27];
}
#endif

#ifndef ff_ctzll
#define ff_ctzll ff_ctzll_c


static inline  int ff_ctzll_c(long long v)
{
    static const uint8_t debruijn_ctz64[64] = {
        0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
        62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
        63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
    };
    return debruijn_ctz64[(uint64_t)((v & -v) * 0x022FDD63CC95386DU) >> 58];
}
#endif

#endif 
