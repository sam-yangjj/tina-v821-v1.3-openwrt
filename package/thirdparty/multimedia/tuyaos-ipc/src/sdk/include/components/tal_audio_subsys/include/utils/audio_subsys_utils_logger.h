/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/utils/audio_subsys_utils_logger.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-07-29 17:34:33
 * @LastEditors: clare.phang
 * @LastEditTime: 2025-01-06 18:56:36
 */
#ifndef __AUDIO_SUBSYS_UTILS_LOGGER_H__
#define __AUDIO_SUBSYS_UTILS_LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_TAG
#define LOG_TAG ""
#endif

/** NOTE: PLS define in makefile
 * x86_64 detach build MUST define.
 * */
#if defined(AUDIO_SUBSYS_UTILS_LOGGER_EXT_ENABLE)

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef filename
#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#endif /** !filename */

#ifndef PR_ERR
#define PR_ERR(format, args...)                                                 \
    printf("[%s][ERROR  ]%s[%s][%s:%d]:" format "\n", GET_TIME_STR(), LOG_TAG,  \
            filename(__FILE__), __func__, (int)__LINE__, ##args);
#endif /** !PR_ERR */

#ifndef PR_WARN
#define PR_WARN(format, args...)                                                \
    printf("[%s][WARNING]%s[%s][%s:%d]:" format "\n", GET_TIME_STR(), LOG_TAG,  \
            filename(__FILE__), __func__, (int)__LINE__, ##args);
#endif /** !PR_WARN */

#ifndef PR_NOTICE
#define PR_NOTICE(format, args...)                                              \
    printf("[%s][NOTICE ]%s[%s][%s:%d]:" format "\n", GET_TIME_STR(), LOG_TAG,  \
            filename(__FILE__), __func__, (int)__LINE__, ##args);
#endif /** !PR_NOTICE */

#ifndef PR_INFO
#define PR_INFO(format, args...)                                                \
    printf("[%s][INFO   ]%s[%s][%s:%d]:" format "\n", GET_TIME_STR(), LOG_TAG,  \
            filename(__FILE__), __func__, (int)__LINE__, ##args);
#endif /** !PR_INFO */

#if !defined(NDEBUG)
#ifndef PR_DEBUG
#define PR_DEBUG(format, args...)                                               \
    printf("[%s][DEBUG  ]%s[%s][%s:%d]:" format "\n", GET_TIME_STR(), LOG_TAG,  \
            filename(__FILE__), __func__, (int)__LINE__, ##args);
#define PR_DEBUG_RAW(format, args...) printf(format, ##args)
#endif /** !PR_DEBUG */
#else /** NDEBUG */
#ifndef PR_DEBUG
#define PR_DEBUG(format, args...)  (void)0
#define PR_DEBUG_RAW(format, args...)  (void)0
#endif /** !PR_DEBUG */
#endif /** !NDEBUG */

#else /** AUDIO_SUBSYS_UTILS_LOGGER_EXT_ENABLE */

/** FIXME: uni_log max printf buffer size < 1024 */
#include "utilities/uni_log.h"

#endif /** !AUDIO_SUBSYS_UTILS_LOGGER_EXT_ENABLE */

#ifndef ASSERT_CHECK
#define ASSERT_CHECK(EXPR)                                                      \
    if (!(EXPR)) {                                                              \
        PR_ERR("ASSERT(%s) has assert failed at %s:%d.",                        \
              #EXPR, __FUNCTION__, __LINE__);                                   \
    }
#endif /** !ASSERT_CHECK */

#ifndef ASSERT_RET
#define ASSERT_RET(EXPR, ret)                                                   \
({                                                                              \
    if (!(EXPR)) {                                                              \
        PR_ERR("ASSERT(%s) has assert failed at %s:%d. ret: %d",                \
              #EXPR, __FUNCTION__, __LINE__, ret);                              \
    }                                                                           \
    ret;                                                                        \
})
#endif /** !ASSERT_RET */

#ifndef ASSERT
#define ASSERT(EXPR)            ASSERT_CHECK(EXPR)
#endif /** !ASSERT */

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_UTILS_LOGGER_H__ */
