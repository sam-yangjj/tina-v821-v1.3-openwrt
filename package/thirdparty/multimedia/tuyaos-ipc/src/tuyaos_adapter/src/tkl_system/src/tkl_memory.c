/**
 * @file tkl_memory.c
 * @brief the default weak implements of tuya hal memory, this implement only used when OS=linux
 * @version 0.1
 * @date 2020-05-15
 * 
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#include "tkl_memory.h"
#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_GET_FREE_HEAP
#include <sys/sysinfo.h>
#endif

/**
* @brief Alloc memory of system
*
* @param[in] size: memory size
*
* @note This API is used to alloc memory of system.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE VOID_T* tkl_system_malloc(CONST SIZE_T size)
{
    return malloc(size);
}

/**
* @brief Free memory of system
*
* @param[in] ptr: memory point
*
* @note This API is used to free memory of system.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE VOID_T tkl_system_free(VOID_T* ptr)
{
    return free(ptr);
}

/**
* @brief set memory
*
* @param[in] size: memory size
*
* @note This API is used to alloc memory of system.
*
* @return the memory address malloced
*/
TUYA_WEAK_ATTRIBUTE VOID_T *tkl_system_memset(VOID_T* src, INT_T ch, CONST SIZE_T n)
{
    return memset(src, ch, n);
}

/**
* @brief Alloc memory of system
*
* @param[in] size: memory size
*
* @note This API is used to alloc memory of system.
*
* @return the memory address malloced
*/
TUYA_WEAK_ATTRIBUTE VOID_T *tkl_system_memcpy(VOID_T* src, CONST VOID_T* dst, CONST SIZE_T n)
{
    return memcpy(src, dst, n);
}

/**
 * @brief Allocate and clear the memory
 * 
 * @param[in]       nitems      the numbers of memory block
 * @param[in]       size        the size of the memory block
 */
TUYA_WEAK_ATTRIBUTE VOID_T *tkl_system_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

/**
 * @brief Re-allocate the memory
 * 
 * @param[in]       nitems      source memory address
 * @param[in]       size        the size after re-allocate
 */
TUYA_WEAK_ATTRIBUTE VOID_T *tkl_system_realloc(VOID_T* ptr, size_t size)
{
    return realloc(ptr, size);
}


/**
 * @brief memory cmpare 
 * 
 * @param[in]       str1     source string  
 * @param[in]       str2     dst string  
 * @param[in]       n        bytes  
 */
INT_T tkl_system_memcmp(CONST VOID_T *str1, CONST VOID_T *str2, SIZE_T n)
{
    return memcmp(str1,str2, n);
}

INT_T tkl_system_get_free_heap_size(VOID_T)
{
#ifdef ENABLE_GET_FREE_HEAP
    struct sysinfo info;
    unsigned long freeram = 0;

    if(sysinfo(&info) == 0) {
       freeram = info.freeram;
    }

    return (INT_T)freeram;
#else
    return 0;
#endif
}
