
#ifndef __TUYA_MUDEX_MATHMATICS_UTIL_H__
#define __TUYA_MUDEX_MATHMATICS_UTIL_H__

#include <stdint.h>
#include <math.h>
#include "tuya_mudex_rational_util.h"
#include "tuya_mudex_intfloat_util.h"

#ifndef M_E
#define M_E            2.7182818284590452354   /* e */
#endif
#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif
#ifndef M_LN10
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#endif
#ifndef M_LOG2_10
#define M_LOG2_10      3.32192809488736234787  /* log_2 10 */
#endif
#ifndef M_PHI
#define M_PHI          1.61803398874989484820   /* phi / golden ratio */
#endif
#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif
#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif
#ifndef NAN
#define NAN            av_int2float(0x7fc00000)
#endif
#ifndef INFINITY
#define INFINITY       av_int2float(0x7f800000)
#endif

/**
 * Rounding methods.
 */
enum AVRounding {
    AV_ROUND_ZERO     = 0, ///< Round toward zero.
    AV_ROUND_INF      = 1, ///< Round away from zero.
    AV_ROUND_DOWN     = 2, ///< Round toward -infinity.
    AV_ROUND_UP       = 3, ///< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
    /**
     * Flag telling rescaling functions to pass `INT64_MIN`/`MAX` through
     * unchanged, avoiding special cases for #TYM_NOPTS_VALUE.
     *
     * Unlike other values of the enumeration AVRounding, this value is a
     * bitmask that must be used in conjunction with another value of the
     * enumeration through a bitwise OR, in order to set behavior for normal
     * cases.
     *
     * @code{.c}
     * av_rescale_rnd(3, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
     * // Rescaling 3:
     * //     Calculating 3 * 1 / 2
     * //     3 / 2 is rounded up to 2
     * //     => 2
     *
     * av_rescale_rnd(TYM_NOPTS_VALUE, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
     * // Rescaling TYM_NOPTS_VALUE:
     * //     TYM_NOPTS_VALUE == INT64_MIN
     * //     TYM_NOPTS_VALUE is passed through
     * //     => TYM_NOPTS_VALUE
     * @endcode
     */
    AV_ROUND_PASS_MINMAX = 8192,
};

/**
 * Compute the greatest common divisor of two integer operands.
 *
 * @param a,b Operands
 * @return GCD of a and b up to sign; if a >= 0 and b >= 0, return value is >= 0;
 * if a == 0 and b == 0, returns 0.
 */
int64_t  av_gcd(int64_t a, int64_t b);

/**
 * Rescale a 64-bit integer with rounding to nearest.
 *
 * The operation is mathematically equivalent to `a * b / c`, but writing that
 * directly can overflow.
 *
 * This function is equivalent to av_rescale_rnd() with #AV_ROUND_NEAR_INF.
 *
 * @see av_rescale_rnd(), av_rescale_q(), av_rescale_q_rnd()
 */
int64_t av_rescale(int64_t a, int64_t b, int64_t c) ;

/**
 * Rescale a 64-bit integer with specified rounding.
 *
 * The operation is mathematically equivalent to `a * b / c`, but writing that
 * directly can overflow, and does not support different rounding methods.
 *
 * @see av_rescale(), av_rescale_q(), av_rescale_q_rnd()
 */
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd) ;

/**
 * Rescale a 64-bit integer by 2 rational numbers.
 *
 * The operation is mathematically equivalent to `a * bq / cq`.
 *
 * This function is equivalent to av_rescale_q_rnd() with #AV_ROUND_NEAR_INF.
 *
 * @see av_rescale(), av_rescale_rnd(), av_rescale_q_rnd()
 */
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) ;

/**
 * Rescale a 64-bit integer by 2 rational numbers with specified rounding.
 *
 * The operation is mathematically equivalent to `a * bq / cq`.
 *
 * @see av_rescale(), av_rescale_rnd(), av_rescale_q()
 */
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
                         enum AVRounding rnd) ;

/**
 * Compare two timestamps each in its own time base.
 *
 * @return One of the following values:
 *         - -1 if `ts_a` is before `ts_b`
 *         - 1 if `ts_a` is after `ts_b`
 *         - 0 if they represent the same position
 *
 * @warning
 * The result of the function is undefined if one of the timestamps is outside
 * the `int64_t` range when represented in the other's timebase.
 */
int av_compare_ts(int64_t ts_a, AVRational tb_a, int64_t ts_b, AVRational tb_b);








/**
 * @}
 */

#endif /* AVUTIL_MATHEMATICS_H */
