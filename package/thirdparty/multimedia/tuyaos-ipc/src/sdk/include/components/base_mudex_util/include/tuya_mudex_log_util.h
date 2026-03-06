#ifndef __TUYA_MUDEXER_UTILS_LOG_H__
#define __TUYA_MUDEXER_UTILS_LOG_H__

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>



void __my_log_print_col(char *level, const char *fmt, ...); 
    
#define MUDEX_DEBUG(fmt, ...)    __my_log_print_col("DEBUG" , fmt, ##__VA_ARGS__)
#define MUDEX_ERR(fmt, ...)    __my_log_print_col("ERR", fmt, ##__VA_ARGS__)
#define MUDEX_WARNING(fmt, ...)    __my_log_print_col("WARNING", fmt, ##__VA_ARGS__)
#define MUDEX_TRACE(fmt, ...)    //printf("Trace:"fmt"\r\n", ##__VA_ARGS__)



//log 
/**
 * Print no output.
 */
#define AV_LOG_QUIET    -8
#define AV_LOG_PANIC     0
#define AV_LOG_FATAL     8
#define AV_LOG_ERROR    16
#define AV_LOG_WARNING  24
#define AV_LOG_INFO     32
#define AV_LOG_VERBOSE  40
#define AV_LOG_DEBUG    48
#define AV_LOG_TRACE    56
#define AV_LOG_C(x) ((x) << 8)
#define tym_log(ctx,flag,fmt, ...) MUDEX_DEBUG(fmt, ##__VA_ARGS__)

/**
 * assert() equivalent, that is always enabled.
 */
#define tuya_muxer_assert0(cond) do {                                           \
    if (!(cond)) {                                                      \
        tym_log(NULL, AV_LOG_PANIC, "Assertion %s failed at %s:%d\n",    \
               cond, __FILE__, __LINE__);                 \
        abort();                                                        \
    }                                                                   \
} while (0)
#define tuya_muxer_assert(cond) tuya_muxer_assert0(cond)

#endif