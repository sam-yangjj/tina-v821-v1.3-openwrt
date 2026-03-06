/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/utils/audio_subsys_utils_debug.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-07-29 19:13:40
 * @LastEditors: clare.phang
 * @LastEditTime: 2025-01-06 18:34:40
 */
#ifndef __AUDIO_SUBSYS_UTILS_DEBUG_H__
#define __AUDIO_SUBSYS_UTILS_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#ifdef NDEBUG

#define AUIDO_FILE_DEBUG_TEST_WRITE(filename, data, dlen)
#define AUDIO_FILE_NAME_ASSEMBLE(prefix, suffix, ext)

#else /** NDEBUG */

#define AUIDO_FILE_DEBUG_TEST_WRITE(filename, data, dlen)                       \
    ({                                                                          \
        int __nwrite = -1;                                                      \
        if (NULL != data && dlen > 0) {                                         \
            FILE *__fp = fopen(filename, "ab+");                                \
            if (NULL != __fp) {                                                 \
                __nwrite = fwrite(data, sizeof(char), dlen, __fp);              \
                fclose(__fp);                                                   \
            }                                                                   \
        }                                                                       \
        __nwrite;                                                               \
    })

#define AUDIO_FILE_NAME_ASSEMBLE_WITH_PATH(pathname, prefix, suffix, ext)       \
    ({                                                                          \
        static char __buf[256 * 2] = {0};                                       \
        snprintf(__buf, sizeof(__buf), "%s/%s_%s.%s", pathname, prefix, suffix, ext);   \
        __buf;                                                                  \
    })

// #define AUDIO_FILE_PATHNAME "/tmp/"
#ifndef AUDIO_FILE_PATHNAME
#define AUDIO_FILE_PATHNAME "./"
#endif /** !AUDIO_FILE_PATHNAME */

#define AUDIO_FILE_NAME_ASSEMBLE(prefix, suffix, ext)                           \
    AUDIO_FILE_NAME_ASSEMBLE_WITH_PATH(AUDIO_FILE_PATHNAME, prefix, suffix, ext)

#endif /** !NDEBUG */

#ifndef GET_CURRENT_TIMESTAMP
#define GET_CURRENT_TIMESTAMP()                                                 \
    ({                                                                          \
        struct timespec ts;                                                     \
        static char __timestamp[32] = {0};                                      \
        clock_gettime(CLOCK_REALTIME, &ts);                                     \
        sprintf(__timestamp, "%lf", ((double)ts.tv_sec + (((double)ts.tv_nsec) / (1000 * 1000 * 1000))));   \
        __timestamp; \
    })
#endif /** !GET_CURRENT_TIMESTAMP */

#ifndef GET_CURRENT_STRFTIME
#define GET_CURRENT_STRFTIME()                                                  \
    ({                                                                          \
        time_t __rawtime;                                                       \
        struct tm *__timeinfo;                                                  \
        static char __timestamp[32];                                            \
        time(&__rawtime);                                                       \
        struct timeval __tv;                                                    \
        gettimeofday(&__tv, NULL);                                              \
        long milliseconds = __tv.tv_usec / 1000;                                \
        __timeinfo = localtime(&__rawtime);                                     \
        strftime(__timestamp, sizeof(__timestamp), "%Y-%m-%d-%H-%M-%S", __timeinfo); \
        sprintf(&__timestamp[19],".%03d", (short)milliseconds);                 \
        __timestamp;                                                            \
    })
#endif /** !GET_CURRENT_STRFTIME */

#ifndef GET_TIME_STR
// #define GET_TIME_STR() GET_CURRENT_TIMESTAMP()
#define GET_TIME_STR() GET_CURRENT_STRFTIME()
#endif /** !GET_TIME_STR */

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif /** !UNUSED */

#ifdef TEST_RTF_EN
#include "utils/audio_subsys_utils_logger.h"
#define TEST_REAL_TIME_FACTOR(process_func, self, in, inlen, out, outlen, actual_duration_ms) ({ \
    clock_t begin__ = clock();                                                  \
    int ret__ = process_func(self, in, inlen, out, outlen);                     \
    clock_t end__ = clock();                                                    \
    double elapsed_ms__ = (double)(end__ - begin__) * 1000 / CLOCKS_PER_SEC;    \
    double rtf__ = elapsed_ms__ / (double)actual_duration_ms;                   \
    PR_DEBUG("Elapsed %.3f ms, Actual duration %.3f ms, Real-Time Factor (RTF): %.3f", elapsed_ms__, actual_duration_ms, rtf__); \
    if (rtf__ > 1.0) PR_WARN("Elapsed %.3f ms, Actual duration %.3f ms, Real-Time Factor (RTF): %.3f > 1.0, process_func is too slow!!!", elapsed_ms__, actual_duration_ms, rtf__); \
    ret__;                                                                      \
})
# ifdef TEST_MODULE_RTF
#  define TEST_REAL_TIME_FACTOR_MODULE TEST_REAL_TIME_FACTOR
# endif /** TEST_MODULE_RTF */
#endif /** TEST_RTF_EN */

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_UTILS_DEBUG_H__ */
