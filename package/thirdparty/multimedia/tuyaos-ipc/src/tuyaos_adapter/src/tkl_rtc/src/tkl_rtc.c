/*
 * Copyright (C) 2021 Ty Inc.
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include "sys_log.h"
#include "tkl_rtc.h"

#ifndef TAG 
#define TAG "tkl_rtc"
#endif

#ifndef MAX_PATH 
#define MAX_PATH 256
#endif

#define RTC_DEFAULT_DEVICE  TY_TKL_RTC_DEV

static int sg_rtc_fd = -1;

OPERATE_RET tkl_rtc_init(VOID_T)
{
    char dev_name[MAX_PATH] = {0};

    strncpy(dev_name, RTC_DEFAULT_DEVICE, MAX_PATH-1);

    sg_rtc_fd = open(dev_name, O_RDWR);

    if (sg_rtc_fd < 0) {
        LOGE(TAG, "Failed to open %s\n", dev_name);
        return OPRT_COM_ERROR;
    }

    // Set the close-on-exec flag for the file descriptor
    if (-1 == fcntl(sg_rtc_fd, F_SETFD, FD_CLOEXEC)) {
        LOGE(TAG, "fcntl error\n");
        close(sg_rtc_fd);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_rtc_deinit(VOID_T)
{
    if (sg_rtc_fd != -1) {
        close(sg_rtc_fd);
        sg_rtc_fd = -1;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_rtc_time_set(TIME_T time_sec)
{
    int ret;
    struct tm tm_utc;
    time_t stamp = (time_t)time_sec;
    struct rtc_time rtc = {0};

    if (gmtime_r(&stamp, &tm_utc) == NULL) {
        LOGE(TAG, "gmtime_r failed\n");
        return OPRT_COM_ERROR;
    }

    LOGD(TAG, "set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                         tm_utc.tm_year + 1900, tm_utc.tm_mon + 1,tm_utc.tm_mday,
			 tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec);

    rtc.tm_year = tm_utc.tm_year;
    rtc.tm_mon  = tm_utc.tm_mon;
    rtc.tm_mday = tm_utc.tm_mday;
    rtc.tm_hour = tm_utc.tm_hour;
    rtc.tm_min  = tm_utc.tm_min;
    rtc.tm_sec  = tm_utc.tm_sec;

    ret = ioctl(sg_rtc_fd, RTC_SET_TIME, &rtc);

    if (ret < 0) {
        LOGE(TAG, "RTC_SET_TIME failed\n");
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_rtc_time_get(TIME_T *time_sec)
{
    struct rtc_time rtc;
    struct tm tm_utc = {0};
    time_t timestamp;
    int ret;

    if (time_sec == NULL) {
        return OPRT_INVALID_PARM;
    }

    ret = ioctl(sg_rtc_fd, RTC_RD_TIME, &rtc);

    if (ret < 0) {
        LOGE(TAG, "RTC_RD_TIME failed\n");
        return OPRT_COM_ERROR;
    }

    tm_utc.tm_year = rtc.tm_year;
    tm_utc.tm_mon =  rtc.tm_mon;
    tm_utc.tm_mday = rtc.tm_mday;
    tm_utc.tm_hour = rtc.tm_hour;
    tm_utc.tm_min =  rtc.tm_min;
    tm_utc.tm_sec =  rtc.tm_sec;

    timestamp = timegm(&tm_utc);

    if (timestamp == (time_t) -1) {
        LOGE(TAG, "timegm failed\n");
        return OPRT_COM_ERROR;
    }

    *time_sec = (TIME_T)timestamp;

    return OPRT_OK;
}
