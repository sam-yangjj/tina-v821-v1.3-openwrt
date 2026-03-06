/**
 * @file tkl_thread.c
 * @brief the default weak implements of tuya os thread
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 * 
 */

#include "tuya_iot_config.h"
#include "tkl_thread.h"
#include "tkl_memory.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <string.h>

#ifdef ENABLE_PRIORITY_SCHED
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#define SCHED_MODE SCHED_RR
#endif

typedef struct {
    pthread_t       id;
    THREAD_FUNC_T   func;
    VOID_T*         arg;
} THREAD_DATA;

STATIC VOID_T* _tkl_thread_wrap_func(VOID_T* arg)
{
    THREAD_DATA* thread_data = (THREAD_DATA*)arg;
    if (thread_data && thread_data->func) {
        thread_data->func(thread_data->arg);
    }

    return NULL;
}

/**
* @brief Create thread
*
* @param[out] thread: thread handle
* @param[in] name: thread name
* @param[in] stack_size: stack size of thread
* @param[in] priority: priority of thread
* @param[in] func: the main thread process function
* @param[in] arg: the args of the func, can be null
*
* @note This API is used for creating thread.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_create(TKL_THREAD_HANDLE* thread,
                           CONST CHAR_T* name,
                           UINT_T stack_size,
                           UINT_T priority,
                           THREAD_FUNC_T func,
                           VOID_T* CONST arg)
{
    INT_T ret = 0;
    pthread_attr_t attr;
    THREAD_DATA* thread_data;

    if (!thread) {
        return OPRT_INVALID_PARM;
    }
    
    thread_data = (THREAD_DATA*)tkl_system_malloc(sizeof(THREAD_DATA));

    if (thread_data == NULL) {
       return OPRT_MALLOC_FAILED;
    }

    memset(thread_data, 0, sizeof(THREAD_DATA));
    // THREAD_DATA thread_data;
    thread_data->id   = 0;
    thread_data->func = func;
    thread_data->arg  = arg;

    pthread_attr_init(&attr);
#if defined (ENABLE_THREAD_STACK_SIZE)
    pthread_attr_setstacksize(&attr, stack_size);
#endif

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

#if defined (ENABLE_PRIORITY_SCHED) && defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && !defined (sun)
    int min_priority,max_priority;
    struct sched_param param = {0};

    ret = pthread_attr_setschedpolicy(&attr, SCHED_MODE);

    if(ret != 0) {
        fprintf(stderr,"Set sched policy fail\n");
        pthread_attr_destroy(&attr);
        return OPRT_COM_ERROR;
    }

    min_priority = sched_get_priority_min(SCHED_MODE);
    max_priority = sched_get_priority_max(SCHED_MODE);

    if(min_priority == -1 || max_priority == -1) {
        fprintf(stderr, "Get min/max priority fail\n");
        pthread_attr_destroy(&attr);
        return OPRT_INVALID_PARM;
    }

    if(priority + min_priority > max_priority) {
        fprintf(stderr, "Priority need in min(%d) ~ max(%d)\n", 0, max_priority - min_priority);
        pthread_attr_destroy(&attr);
        return OPRT_INVALID_PARM;
    }

    param.sched_priority = priority + min_priority;
    ret = pthread_attr_setschedparam(&attr, &param);
    ret |= pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    if(ret != 0) {
        fprintf(stderr, "Pthread attr set fail\n");
        pthread_attr_destroy(&attr);
        return OPRT_COM_ERROR;
    }

#endif

    ret = pthread_create(&(thread_data->id) ,&attr, _tkl_thread_wrap_func, thread_data);

    pthread_attr_destroy(&attr);
    if (0 != ret) {
        return OPRT_OS_ADAPTER_THRD_CREAT_FAILED;
    }
        
    *thread = (TKL_THREAD_HANDLE)thread_data;

    return OPRT_OK;
}

/**
* @brief Terminal thread and release thread resources
*
* @param[in] thread: thread handle
*
* @note This API is used to terminal thread and release thread resources.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_release(TKL_THREAD_HANDLE thread)
{
    if (!thread) {
        return OPRT_INVALID_PARM;
    }
    
    THREAD_DATA* thread_data = (THREAD_DATA*)thread;
    tkl_system_free(thread_data);
    
    return OPRT_OK;
}

/**
* @brief Get the thread stack's watermark
*
* @param[in] thread: thread handle
* @param[out] watermark: watermark in Bytes
*
* @note This API is used to get the thread stack's watermark.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*
*/
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_get_watermark(CONST TKL_THREAD_HANDLE thread, UINT_T* watermark)
{
    *watermark = -1;
    return OPRT_OK;
}

/**
* @brief Get the thread thread handle
*
* @param[out] thread: thread handle
*
* @note This API is used to get the thread handle.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_get_id(TKL_THREAD_HANDLE *thread)
{
    pthread_t pid = pthread_self();
    *thread = (TKL_THREAD_HANDLE)pid;
    
    return OPRT_OK;
}


/**
* @brief Set name of self thread
*
* @param[in] name: thread name
*
* @note This API is used to set name of self thread.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_set_self_name(CONST CHAR_T* name)
{
    if (!name) {
        return OPRT_INVALID_PARM;
    }

    prctl(PR_SET_NAME, name);
    return OPRT_OK;
}



TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_is_self(TKL_THREAD_HANDLE thread, BOOL_T* is_self)
{
    if (NULL == thread || NULL == is_self) {
        return OPRT_INVALID_PARM;
    }

    THREAD_DATA* thread_data = (THREAD_DATA*)thread;

    *is_self = pthread_equal(thread_data->id, pthread_self());

    return OPRT_OK;
}

OPERATE_RET tkl_thread_get_priority(TKL_THREAD_HANDLE thread, INT_T *priority)
{
#if defined (ENABLE_PRIORITY_SCHED) && defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && !defined (sun)
    int policy;
    OPERATE_RET iret = OPRT_COM_ERROR;
    struct sched_param param = {0};

    if (NULL == thread || NULL == priority) {
        return OPRT_INVALID_PARM;
    }

    if(pthread_getschedparam((pthread_t)thread, &policy, &param) == 0 && policy == SCHED_MODE) {
        *priority = param.sched_priority;
	iret = OPRT_OK;
    }

    return iret;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

OPERATE_RET tkl_thread_set_priority(TKL_THREAD_HANDLE thread, INT_T priority)
{
#if defined (ENABLE_PRIORITY_SCHED) && defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && !defined (sun)

    OPERATE_RET iret = OPRT_OK;
    int min_priority,max_priority;
    struct sched_param param = {0};

    if(NULL == thread) {
        return OPRT_INVALID_PARM;
    }

    if(geteuid() !=0) {
        return OPRT_NOT_SUPPORTED;
    }

    min_priority = sched_get_priority_min(SCHED_MODE);
    max_priority = sched_get_priority_max(SCHED_MODE);

    if(min_priority == -1 || max_priority == -1) {
        fprintf(stderr, "Get min/max priority fail\n");
	return OPRT_COM_ERROR;
    }

    if(priority + min_priority > max_priority) {
        fprintf(stderr, "Priority need in min(%d) ~ max(%d)\n", 0, max_priority - min_priority);
	return OPRT_COM_ERROR;
    }

    param.sched_priority = priority + min_priority;

    if(pthread_setschedparam((pthread_t)thread, SCHED_MODE, &param) != 0) {
        fprintf(stderr, "Setschedparam fail\n");
	iret = OPRT_COM_ERROR;
    }

    return iret;
#else
    return OPRT_NOT_SUPPORTED;
#endif
}

TUYA_WEAK_ATTRIBUTE OPERATE_RET tkl_thread_diagnose(TKL_THREAD_HANDLE thread)
{
    return OPRT_NOT_SUPPORTED;
}
