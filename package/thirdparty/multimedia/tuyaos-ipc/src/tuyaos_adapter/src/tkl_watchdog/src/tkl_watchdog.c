/*
 * Copyright (C) 2021 Tuya Inc.
 */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include "sys_log.h"
#include "tkl_watchdog.h"

#ifndef TAG 
#define TAG "tkl_wdt"
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define WATCHDOG_DEFAULT_DEVICE TUYA_TKL_WDT_DEV

static int sg_wdt_fd = -1;

UINT32_T tkl_watchdog_init(TUYA_WDOG_BASE_CFG_T *cfg)
{
    int fd = -1;
    int timeout; // in seconds
    char dev_name[MAX_PATH] = {0};

    if(sg_wdt_fd != -1) {
	LOGE(TAG, "watchdog is inited\n");
        return 0;
    }

    if (cfg == NULL) {
	LOGE(TAG, "invalid param\n");
        return 0;
    }

    strncpy(dev_name, WATCHDOG_DEFAULT_DEVICE, MAX_PATH-1);

    fd = open(dev_name, O_RDWR);

    if (fd < 0) {
        LOGE(TAG, "Failed to open %s\n", dev_name);
        return 0;
    }

    timeout = (int)cfg->interval_ms / 1000;

    if(ioctl(fd, WDIOC_SETTIMEOUT, &timeout)) {
        LOGE(TAG, "Failed to set watchdog timeout\n");
	close(fd);
	return 0;
    }

    if(ioctl(fd, WDIOC_GETTIMEOUT, &timeout)) {
        LOGE(TAG, "Failed to get watchdog timeout\n");
        close(fd);
        return 0;
    }

    sg_wdt_fd = fd;

    return (UINT_T)(timeout * 1000);
}

OPERATE_RET tkl_watchdog_deinit(VOID_T)
{
    char magic = 'V';
    int ret = 0;
    int option = WDIOS_DISABLECARD;

    if(sg_wdt_fd == -1) {
        LOGE(TAG, "watchdog not initialized\n");
        return OPRT_COM_ERROR;
    }

    /*
     * A magic character 'V' must be written before closing the watchdog device.
     * Otherwise, the watchdog will reset the system after closing fd.
     */
    ret = write(sg_wdt_fd, &magic, sizeof(magic));

    if (ret < 0) {
        LOGE(TAG, "write magic word failed\n");
    }

    ret = ioctl(sg_wdt_fd, WDIOC_SETOPTIONS, &option);

    if(ret < 0) {
        LOGE(TAG, "WDIOC_SETOPTIONS failed\n");
    }

    close(sg_wdt_fd);

    sg_wdt_fd = -1;

    return OPRT_OK;
}

OPERATE_RET tkl_watchdog_refresh(VOID_T)
{
    OPERATE_RET ret = OPRT_OK;

    if(ioctl(sg_wdt_fd, WDIOC_KEEPALIVE, NULL)) {
	LOGE(TAG, "WDIOC_KEEPALIVE fail\n");
        ret = OPRT_COM_ERROR;
    }

    return ret;
}
