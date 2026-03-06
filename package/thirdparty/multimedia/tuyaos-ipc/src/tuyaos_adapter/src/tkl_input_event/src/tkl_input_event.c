/*
 * Copyright (C) 2021 Tuya Inc.
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <dirent.h>
#include "sys_log.h"
#include "tkl_input_event.h"

#ifndef TAG 
#define TAG "tkl_input_event"
#endif

#define INPUT_EVENT_DEVICE "/dev/input"
#define INPUT_EVENT_NODE "event"
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)

static int sg_input_epoll_fd = -1;
static int sg_input_device_fd[MAX_DEVICES] = {-1};
static unsigned int sg_input_device_count = 0;

static int __test_bit(size_t bit, unsigned long* array)
{
    return (array[bit/BITS_PER_LONG] & (1UL << (bit % BITS_PER_LONG))) != 0;
}

static int __get_input_event(struct epoll_event* poll_event, struct input_event* ev)
{
    int iret = -1;
    ssize_t result;

    if(!poll_event || !ev) {
        LOGE(TAG, "invalid argument\n");
        return -1;
    }

    if(!(poll_event->events&EPOLLIN)) { 
        LOGE(TAG, "invalid event\n");
        return -1;
    }

    do {
        result = read(poll_event->data.fd, ev, sizeof(*ev));
    }while(result == -1 && errno == EINTR);

    if(result == sizeof(*ev)) {
        iret = 0; 
    }

    return iret;
}

OPERATE_RET tkl_input_event_init(VOID_T)
{
    int efd;
    DIR* dir;
    struct dirent* de;

    if (sg_input_epoll_fd != -1) {
        LOGE(TAG, "input_event has already init\n");
        return OPRT_OK;
    }

    efd = epoll_create(MAX_DEVICES);

    if (efd < 0) {
	LOGE(TAG, "epoll_create fail\n");
        return OPRT_COM_ERROR;
    }

    sg_input_epoll_fd  = efd;

    dir = opendir(INPUT_EVENT_DEVICE);

    if(dir == NULL) {
        LOGE(TAG, "open dir:%s fail\n", INPUT_EVENT_DEVICE);
    }

    /* try to open all input device */

    while ((de = readdir(dir))) {
        int fd;
        struct epoll_event ev;
        unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];

        if(strncmp(de->d_name, INPUT_EVENT_NODE, 5))
            continue;

        fd = openat(dirfd(dir), de->d_name, O_RDONLY);

        if(fd < 0)
            continue;

        /* Read the evbits of the input device.*/
        if(ioctl(fd, EVIOCGBIT(0, EV_MAX), ev_bits) < 0) {
             close(fd);
             continue;
        }

        /* We assume that only EV_KEY, EV_REL, and EV_SW event types are ever needed.*/
        if(!__test_bit(EV_KEY, ev_bits) && !__test_bit(EV_REL, ev_bits) && 
            !__test_bit(EV_SW, ev_bits) && !__test_bit(EV_ABS, ev_bits)) {
            close(fd);
            continue;
        }

        ev.events = EPOLLIN;
        ev.data.fd = fd;

        if(epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) < 0) {
             close(fd);
             LOGE(TAG, "epoll_ctl failed errno:%d\n", -errno);
             continue;
        }

        sg_input_device_fd[sg_input_device_count] = fd;
        sg_input_device_count++;

        if (sg_input_device_count == MAX_DEVICES)
            break;
    }

    closedir(dir);


    if (!sg_input_device_count) {
        close(sg_input_epoll_fd);
        sg_input_epoll_fd = -1;
        LOGE(TAG, "can't find input device\n");
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_input_event_recv(TUYA_INPUT_EVENT_T *in_evs, PINT_T ev_cnts, INT_T ms)
{
    int i;
    int total = 0;
    struct epoll_event* evs;

    if (in_evs == NULL || ev_cnts == NULL) {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    evs = malloc(sizeof(struct epoll_event) * MAX_DEVICES);

    if(evs == NULL) {
        LOGE(TAG, "malloc fail\n");
        return OPRT_COM_ERROR;
    }

    memset(evs,0,sizeof(struct epoll_event) * MAX_DEVICES);

    total = epoll_wait(sg_input_epoll_fd, evs, MAX_DEVICES,ms);

    if (total <= 0) {
        free(evs);
        return OPRT_COM_ERROR;
    }
     
    for (i = 0; i < total; i++) {
        int ret;
        int cnts = 0;
        struct input_event ev;

        memset(&ev, 0, sizeof(struct input_event));

        ret = __get_input_event(&evs[i], &ev);

        if (ret < 0) {
            /* Ignore EV_SYN event */
            if (ret == -EAGAIN) {
                 continue;
             }
             LOGE(TAG, "get_input_event failed ret:%d\n", ret);
             free(evs);
             return OPRT_COM_ERROR;
        }

        /*make sure struct is same*/
        memcpy(&in_evs[cnts], (TUYA_INPUT_EVENT_T*)&ev, sizeof(TUYA_INPUT_EVENT_T));

        cnts++;
        *ev_cnts = cnts;
    }

    free(evs);

    return OPRT_OK;
}

VOID_T tkl_input_event_deinit(VOID_T)
{
    int total = sg_input_device_count;

    while (total > 0) {
        close(sg_input_device_fd[--total]);
    }

    sg_input_device_count = 0;

    if(sg_input_epoll_fd > 0) {
        close(sg_input_epoll_fd);
        sg_input_epoll_fd = -1;
    }

    return ;
}
