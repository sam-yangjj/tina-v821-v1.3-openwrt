/*
 * Copyright (C) 2021 Ty Inc.
 */
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include "sys_log.h"
#include "tkl_file.h"

#ifndef TAG 
#define TAG "tkl_file"
#endif

int tkl_file_read(char *path, void *buf, int size)
{
    int fd;
    int ret;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE(TAG, "Failed to open %s\n", path);
	return OPRT_INVALID_PARM;
    }

    ret = read(fd, buf, size);
    if (ret < 0) {
        LOGE(TAG, "Failed to read %s\n", path);
        close(fd);
        return OPRT_COM_ERROR;
    }

    close(fd);

    return ret;
}

int tkl_file_write(char *path, void *buf, int size)
{
    int fd;
    int ret;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE(TAG, "Failed to open %s\n", path);
	return OPRT_INVALID_PARM;
    }

    ret = write(fd, buf, size);
    if (ret < 0) {
        LOGE(TAG, "Failed to write %s\n", path);
        close(fd);
        return OPRT_COM_ERROR;
    }

    close(fd);
    return ret;
}

int tkl_file_exists(char *path)
{
    struct stat file_stat;

    return (stat(path, &file_stat) == 0);
}

OPERATE_RET tkl_file_size(char *path, int *size)
{
    struct stat file_stat;

    if (NULL == size) {
	return OPRT_INVALID_PARM;
    }

    if (stat(path, &file_stat) != 0) {
        return OPRT_COM_ERROR;
    }

    *size = file_stat.st_size;

    return OPRT_OK;
}
