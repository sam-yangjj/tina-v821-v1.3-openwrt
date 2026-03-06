/*
 * @Description: file content
 * @FilePath: /tal_audio_subsys/include/utils/audio_subsys_utils_memory.h
 * @Version: v0.0.0
 * @Company: ClarePhang
 * @Author: clare.phang <clare.phang@foxmail.com>
 * @Date: 2024-07-29 17:34:33
 * @LastEditors: clare.phang
 * @LastEditTime: 2024-07-29 19:16:17
 */
#ifndef __AUDIO_SUBSYS_UTILS_MEMORY_H__
#define __AUDIO_SUBSYS_UTILS_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

/** NOTE: PLS define in makefile
 * x86_64 detach build MUST define.
 * */
#if defined(AUDIO_SUBSYS_UTILS_MEMORY_EXT_ENABLE)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MALLOC
#define MALLOC(size) malloc(size)
#endif /** !MALLOC */

#ifndef CALLOC
#define CALLOC(nmemb, size) calloc(nmemb, size)
#endif /** !CALLOC */

#ifndef FREE
#define FREE(ptr) free(ptr)
#endif /** !FREE */

#ifndef MEMSET
#define MEMSET(s, c, n) memset(s, c, n)
#endif /** !MEMSET */

#ifndef MEMCPY
#define MEMCPY(dest, src, n) memcpy(dest, src, n)
#endif /** !MEMCPY */

#ifndef MEMCMP
#define MEMCMP(s1, s2, n) memcmp(s1, s2, n)
#endif /** !MEMCMP */

#ifndef REALLOC
#define REALLOC(ptr, size) realloc(ptr, size)
#endif /** !REALLOC */

#else /** AUDIO_SUBSYS_UTILS_MEMORY_EXT_ENABLE */

// #include "tuya_iot_config.h"
//#include "mem_pool.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MALLOC
#define MALLOC(size) Malloc(size)
#endif /** !MALLOC */

#ifndef CALLOC
#define CALLOC(nmemb, size) Calloc(nmemb, size)
#endif /** !CALLOC */

#ifndef FREE
#define FREE(ptr) Free(ptr)
#endif /** !FREE */

#if defined(ENABLE_TUYAOS) && ENABLE_TUYAOS==1

#ifndef MEMSET
#define MEMSET(s, c, n) tal_system_memset(s, c, n)
#endif /** !MEMSET */

#ifndef MEMCPY
#define MEMCPY(dest, src, n) tal_system_memcpy(dest, src, n)
#endif /** !MEMCPY */

#ifndef MEMCMP
#define MEMCMP(s1, s2, n) tal_system_memcmp(s1, s2, n)
#endif /** !MEMCMP */

#ifndef REALLOC
#define REALLOC(ptr, size) tal_realloc(ptr, size)
#endif /** !REALLOC */

#else /** ENABLE_TUYAOS */

#ifndef MEMSET
#define MEMSET(s, c, n) memset(s, c, n)
#endif /** !MEMSET */

#ifndef MEMCPY
#define MEMCPY(dest, src, n) memcpy(dest, src, n)
#endif /** !MEMCPY */

#ifndef MEMCMP
#define MEMCMP(s1, s2, n) memcmp(s1, s2, n)
#endif /** !MEMCMP */

#ifndef REALLOC
#define REALLOC(ptr, size) realloc(ptr, size)
#endif /** !REALLOC */

#endif /** !ENABLE_TUYAOS */

#endif /** !AUDIO_SUBSYS_UTILS_MEMORY_EXT_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /** !__AUDIO_SUBSYS_UTILS_MEMORY_H__  */
