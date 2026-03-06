

#ifndef AVCODEC_GET_BITS_H
#define AVCODEC_GET_BITS_H

#include <stdint.h>
#include "tuya_mudex_io_internal.h"
#include "tuya_mudex_util_common.h"
#include "tuya_mudex_intreadwrite_util.h"

#include "tuya_mudex_log_util.h"
#include "tuya_mudex_error_util.h"
//#include "avcodec.h"
//#include "mathops.h"



#ifndef CACHED_BITSTREAM_READER
#define CACHED_BITSTREAM_READER 1
#endif

typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
#if CACHED_BITSTREAM_READER
    uint64_t cache;
    unsigned bits_left;
#endif
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

static inline unsigned int get_bits(GetBitContext *s, int n);
static inline void skip_bits(GetBitContext *s, int n);
static inline unsigned int show_bits(GetBitContext *s, int n);


#if CACHED_BITSTREAM_READER
#   define MIN_CACHE_BITS 64
#elif defined LONG_BITSTREAM_READER
#   define MIN_CACHE_BITS 32
#else
#   define MIN_CACHE_BITS 25
#endif



static inline int get_bits_count(const GetBitContext *s)
{
#if CACHED_BITSTREAM_READER
    return s->index - s->bits_left;
#else
    return s->index;
#endif
}

#if CACHED_BITSTREAM_READER
static inline void refill_32(GetBitContext *s, int is_le)
{
#if !UNCHECKED_BITSTREAM_READER
    if (s->index >> 3 >= s->buffer_end - s->buffer)
        return;
#endif

    if (is_le)
        s->cache = (uint64_t)AV_RL32(s->buffer + (s->index >> 3)) << s->bits_left | s->cache;
    else
        s->cache = s->cache | (uint64_t)AV_RB32(s->buffer + (s->index >> 3)) << (32 - s->bits_left);
    s->index     += 32;
    s->bits_left += 32;
}

static inline void refill_64(GetBitContext *s, int is_le)
{
#if !UNCHECKED_BITSTREAM_READER
    if (s->index >> 3 >= s->buffer_end - s->buffer)
        return;
#endif

    if (is_le)
        s->cache = AV_RL64(s->buffer + (s->index >> 3));
    else
        s->cache = AV_RB64(s->buffer + (s->index >> 3));
    s->index += 64;
    s->bits_left = 64;
}

static inline uint64_t get_val(GetBitContext *s, unsigned n, int is_le)
{
    uint64_t ret;
    tuya_muxer_assert(n>0 && n<=63);
    if (is_le) {
        ret = s->cache & ((UINT64_C(1) << n) - 1);
        s->cache >>= n;
    } else {
        ret = s->cache >> (64 - n);
        s->cache <<= n;
    }
    s->bits_left -= n;
    return ret;
}

static inline unsigned show_val(const GetBitContext *s, unsigned n)
{
#ifdef BITSTREAM_READER_LE
    return s->cache & ((UINT64_C(1) << n) - 1);
#else
    return s->cache >> (64 - n);
#endif
}
#endif

/**
 * Skips the specified number of bits.
 * @param n the number of bits to skip,
 *          For the UNCHECKED_BITSTREAM_READER this must not cause the distance
 *          from the start to overflow int32_t. Staying within the bitstream + padding
 *          is sufficient, too.
 */
static inline void skip_bits_long(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    skip_bits(s, n);
#endif
}

#if CACHED_BITSTREAM_READER
static inline void skip_remaining(GetBitContext *s, unsigned n)
{
#ifdef BITSTREAM_READER_LE
    s->cache >>= n;
#else
    s->cache <<= n;
#endif
    s->bits_left -= n;
}
#endif

/**
 * Read MPEG-1 dc-style VLC (sign bit + mantissa with no MSB).
 * if MSB not set it is negative
 * @param n length in bits
 */
static inline int get_xbits(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    int32_t cache = show_bits(s, 32);
    int sign = ~cache >> 31;
    skip_remaining(s, n);

    return ((((uint32_t)(sign ^ cache)) >> (32 - n)) ^ sign) - sign;
#endif
}
#ifndef sign_extend
static inline  int sign_extend(int val, unsigned bits)
{
    unsigned shift = 8 * sizeof(int) - bits;
    union { unsigned u; int s; } v = { (unsigned) val << shift };
    return v.s >> shift;
}
#endif

static inline int get_sbits(GetBitContext *s, int n)
{
    register int tmp;
#if CACHED_BITSTREAM_READER
    tuya_muxer_assert(n>0 && n<=25);
    tmp = sign_extend(get_bits(s, n), n);
#endif
    return tmp;
}

/**
 * Read 1-25 bits.
 */
static inline unsigned int get_bits(GetBitContext *s, int n)
{
    register unsigned int tmp;
#if CACHED_BITSTREAM_READER

    tuya_muxer_assert(n>0 && n<=32);
    if (n > s->bits_left) {
#ifdef BITSTREAM_READER_LE
        refill_32(s, 1);
#else
        refill_32(s, 0);
#endif
        if (s->bits_left < 32)
            s->bits_left = n;
    }

#ifdef BITSTREAM_READER_LE
    tmp = get_val(s, n, 1);
#else
    tmp = get_val(s, n, 0);
#endif
#endif
    tuya_muxer_assert(tmp < UINT64_C(1) << n);
    return tmp;
}

/**
 * Read 0-25 bits.
 */
static inline int get_bitsz(GetBitContext *s, int n)
{
    return n ? get_bits(s, n) : 0;
}

static inline unsigned int get_bits_le(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    tuya_muxer_assert(n>0 && n<=32);
    if (n > s->bits_left) {
        refill_32(s, 1);
        if (s->bits_left < 32)
            s->bits_left = n;
    }

    return get_val(s, n, 1);
#endif
}

/**
 * Show 1-25 bits.
 */
static inline unsigned int show_bits(GetBitContext *s, int n)
{
    register unsigned int tmp;
#if CACHED_BITSTREAM_READER
    if (n > s->bits_left)
#ifdef BITSTREAM_READER_LE
        refill_32(s, 1);
#else
        refill_32(s, 0);
#endif

    tmp = show_val(s, n);
#endif
    return tmp;
}

static inline void skip_bits(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    if (n < s->bits_left)
        skip_remaining(s, n);
    else {
        n -= s->bits_left;
        s->cache = 0;
        s->bits_left = 0;

        if (n >= 64) {
            unsigned skip = (n / 8) * 8;

            n -= skip;
            s->index += skip;
        }
#ifdef BITSTREAM_READER_LE
        refill_64(s, 1);
#else
        refill_64(s, 0);
#endif
        if (n)
            skip_remaining(s, n);
    }
#endif
}

static inline unsigned int get_bits1(GetBitContext *s)
{
#if CACHED_BITSTREAM_READER
    if (!s->bits_left)
#ifdef BITSTREAM_READER_LE
        refill_64(s, 1);
#else
        refill_64(s, 0);
#endif

#ifdef BITSTREAM_READER_LE
    return get_val(s, 1, 1);
#else
    return get_val(s, 1, 0);
#endif

#endif
}

static inline unsigned int show_bits1(GetBitContext *s)
{
    return show_bits(s, 1);
}

static inline void skip_bits1(GetBitContext *s)
{
    skip_bits(s, 1);
}

/**
 * Read 0-32 bits.
 */
static inline unsigned int get_bits_long(GetBitContext *s, int n)
{
    tuya_muxer_assert(n>=0 && n<=32);
    if (!n) {
        return 0;
#if CACHED_BITSTREAM_READER
    }
    return get_bits(s, n);

#endif

}

/**
 * Read 0-64 bits.
 */
static inline uint64_t get_bits64(GetBitContext *s, int n)
{
    if (n <= 32) {
        return get_bits_long(s, n);
    } else {
#ifdef BITSTREAM_READER_LE
        uint64_t ret = get_bits_long(s, 32);
        return ret | (uint64_t) get_bits_long(s, n - 32) << 32;
#else
        uint64_t ret = (uint64_t) get_bits_long(s, n - 32) << 32;
        return ret | get_bits_long(s, 32);
#endif
    }
}

/**
 * Read 0-32 bits as a signed integer.
 */
// static inline int get_sbits_long(GetBitContext *s, int n)
// {
    // if (!n)
        // return 0;
// 
    // return sign_extend(get_bits_long(s, n), n);
// }
// 
/**
 * Show 0-32 bits.
 */
static inline unsigned int show_bits_long(GetBitContext *s, int n)
{
    if (n <= MIN_CACHE_BITS) {
        return show_bits(s, n);
    } else {
        GetBitContext gb = *s;
        return get_bits_long(&gb, n);
    }
}

// static inline int check_marker(void *logctx, GetBitContext *s, const char *msg)
// {
    // int bit = get_bits1(s);
    // if (!bit)
        // tym_log(logctx, AV_LOG_INFO, "Marker bit missing at %d of %d %s\n",
            //    get_bits_count(s) - 1, s->size_in_bits, msg);
// 
    // return bit;
// }

static inline int init_get_bits_xe(GetBitContext *s, const uint8_t *buffer,
                                   int bit_size, int is_le)
{
    int buffer_size;
    int ret = 0;

    if (bit_size >= INT_MAX - TY_MUDEX_MAX(7, TYM_INPUT_BUFFER_PADDING_SIZE*8) || bit_size < 0 || !buffer) {
        bit_size    = 0;
        buffer      = NULL;
        ret         = AVERROR_INVALIDDATA;
    }

    buffer_size = (bit_size + 7) >> 3;

    s->buffer             = buffer;
    s->size_in_bits       = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    s->buffer_end         = buffer + buffer_size;
    s->index              = 0;

#if CACHED_BITSTREAM_READER
    s->cache              = 0;
    s->bits_left          = 0;
    refill_64(s, is_le);
#endif

    return ret;
}

/**
 * Initialize GetBitContext.
 * @param buffer bitstream buffer, must be TYM_INPUT_BUFFER_PADDING_SIZE bytes
 *        larger than the actual read bits because some optimized bitstream
 *        readers read 32 or 64 bit at once and could read over the end
 * @param bit_size the size of the buffer in bits
 * @return 0 on success, AVERROR_INVALIDDATA if the buffer_size would overflow.
 */
static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer,
                                int bit_size)
{
#ifdef BITSTREAM_READER_LE
    return init_get_bits_xe(s, buffer, bit_size, 1);
#else
    return init_get_bits_xe(s, buffer, bit_size, 0);
#endif
}

/**
 * Initialize GetBitContext.
 * @param buffer bitstream buffer, must be TYM_INPUT_BUFFER_PADDING_SIZE bytes
 *        larger than the actual read bits because some optimized bitstream
 *        readers read 32 or 64 bit at once and could read over the end
 * @param byte_size the size of the buffer in bytes
 * @return 0 on success, AVERROR_INVALIDDATA if the buffer_size would overflow.
 */
static inline int init_get_bits8(GetBitContext *s, const uint8_t *buffer,
                                 int byte_size)
{
    if (byte_size > INT_MAX / 8 || byte_size < 0)
        byte_size = -1;
    return init_get_bits(s, buffer, byte_size * 8);
}

// static inline int init_get_bits8_le(GetBitContext *s, const uint8_t *buffer,
                                    // int byte_size)
// {
    // if (byte_size > INT_MAX / 8 || byte_size < 0)
        // byte_size = -1;
    // return init_get_bits_xe(s, buffer, byte_size * 8, 1);
// }

static inline const uint8_t *align_get_bits(GetBitContext *s)
{
    int n = -get_bits_count(s) & 7;
    if (n)
        skip_bits(s, n);
    return s->buffer + (s->index >> 3);
}



static inline int get_bits_left(GetBitContext *gb)
{
    return gb->size_in_bits - get_bits_count(gb);
}

// static inline int skip_1stop_8data_bits(GetBitContext *gb)
// {
    // if (get_bits_left(gb) <= 0)
        // return AVERROR_INVALIDDATA;
// 
    // while (get_bits1(gb)) {
        // skip_bits(gb, 8);
        // if (get_bits_left(gb) <= 0)
            // return AVERROR_INVALIDDATA;
    // }
// 
    // return 0;
// }

#endif /* AVCODEC_GET_BITS_H */
