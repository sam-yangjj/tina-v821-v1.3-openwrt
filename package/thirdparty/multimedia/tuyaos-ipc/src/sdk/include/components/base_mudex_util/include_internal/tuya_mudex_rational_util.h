

#ifndef __TUYA_MUDEX_RATIONAL_UTIL_H__
#define __TUYA_MUDEX_RATIONAL_UTIL_H__
#include "tuya_mudex_util_common.h"
#include "tuya_mudex_log_util.h"
#include <stdint.h>
#include <limits.h>

typedef struct AVRational{
    int num; ///< Numerator
    int den; ///< Denominator
} AVRational;


/**
 * Compare two rationals.
 *
 * @param a First rational
 * @param b Second rational
 *
 * @return One of the following values:
 *         - 0 if `a == b`
 *         - 1 if `a > b`
 *         - -1 if `a < b`
 *         - `INT_MIN` if one of the values is of the form `0 / 0`
 */
static inline int av_cmp_q(AVRational a, AVRational b){
    const int64_t tmp= a.num * (int64_t)b.den - b.num * (int64_t)a.den;

    if(tmp) return (int)((tmp ^ a.den ^ b.den)>>63)|1;
    else if(b.den && a.den) return 0;
    else if(a.num && b.num) return (a.num>>31) - (b.num>>31);
    else                    return INT_MIN;
}

/**
 * Convert an AVRational to a `double`.
 * @param a AVRational to convert
 * @return `a` in floating-point form
 * @see av_d2q()
 */
static inline double av_q2d(AVRational a){
    return a.num / (double) a.den;
}

/**
 * Reduce a fraction.
 *
 * This is useful for framerate calculations.
 *
 * @param[out] dst_num Destination numerator
 * @param[out] dst_den Destination denominator
 * @param[in]      num Source numerator
 * @param[in]      den Source denominator
 * @param[in]      max Maximum allowed values for `dst_num` & `dst_den`
 * @return 1 if the operation is exact, 0 otherwise
 */
int av_reduce(int *dst_num, int *dst_den, int64_t num, int64_t den, int64_t max);

/**
 * Multiply two rationals.
 * @param b First rational
 * @param c Second rational
 * @return b*c
 */
AVRational av_mul_q(AVRational b, AVRational c) ;





/**
 * Invert a rational.
 * @param q value
 * @return 1 / q
 */
static inline AVRational av_inv_q(AVRational q)
{
    AVRational r = { q.den, q.num };
    return r;
}





/**
 * @}
 */

#endif /* AVUTIL_RATIONAL_H */
