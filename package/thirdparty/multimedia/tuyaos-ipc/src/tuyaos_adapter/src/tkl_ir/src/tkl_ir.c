/*
 * Copyright (C) 2021 Ty Inc.
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include "sys_log.h"
#include "tkl_ir.h"

#ifndef TAG 
#define TAG "tkl_ir"
#endif

/*IR ID define*/
#define IR_LIRC TY_TKL_IR_DEV_PATTERN

/*macro from lirc.h*/
#define PULSE_BIT       0x01000000
#define PULSE_MASK      0x00FFFFFF

#define LIRC_MODE2_SPACE     0x00000000
#define LIRC_MODE2_PULSE     0x01000000
#define LIRC_MODE2_FREQUENCY 0x02000000
#define LIRC_MODE2_TIMEOUT   0x03000000

#define LIRC_VALUE_MASK      0x00FFFFFF
#define LIRC_MODE2_MASK      0xFF000000

#define LIRC_SPACE(val) (((val)&LIRC_VALUE_MASK) | LIRC_MODE2_SPACE)
#define LIRC_PULSE(val) (((val)&LIRC_VALUE_MASK) | LIRC_MODE2_PULSE)
#define LIRC_FREQUENCY(val) (((val)&LIRC_VALUE_MASK) | LIRC_MODE2_FREQUENCY)
#define LIRC_TIMEOUT(val) (((val)&LIRC_VALUE_MASK) | LIRC_MODE2_TIMEOUT)

#define LIRC_VALUE(val) ((val)&LIRC_VALUE_MASK)
#define LIRC_MODE2(val) ((val)&LIRC_MODE2_MASK)

#define LIRC_IS_SPACE(val) (LIRC_MODE2(val) == LIRC_MODE2_SPACE)
#define LIRC_IS_PULSE(val) (LIRC_MODE2(val) == LIRC_MODE2_PULSE)
#define LIRC_IS_FREQUENCY(val) (LIRC_MODE2(val) == LIRC_MODE2_FREQUENCY)
#define LIRC_IS_TIMEOUT(val) (LIRC_MODE2(val) == LIRC_MODE2_TIMEOUT)

#define lirc_t int
/*end of macro from lirc.h*/
/* Don't modify this IR max duration limit, because kernel have a same limitation */
#define IR_MAX_DURATION         500000000    /* 500 ms */
typedef unsigned int __u32;

/* IOCTL commands for lirc driver */
#define LIRC_GET_FEATURES              _IOR('i', 0x00000000, __u32)
#define LIRC_GET_SEND_MODE             _IOR('i', 0x00000001, __u32)
#define LIRC_GET_REC_MODE              _IOR('i', 0x00000002, __u32)
#define LIRC_GET_REC_RESOLUTION        _IOR('i', 0x00000007, __u32)
#define LIRC_GET_MIN_TIMEOUT           _IOR('i', 0x00000008, __u32)
#define LIRC_GET_MAX_TIMEOUT           _IOR('i', 0x00000009, __u32)
/* Note: these can reset the according pulse_width */
#define LIRC_SET_SEND_CARRIER          _IOW('i', 0x00000013, __u32)
#define LIRC_SET_REC_CARRIER           _IOW('i', 0x00000014, __u32)
#define LIRC_SET_SEND_DUTY_CYCLE       _IOW('i', 0x00000015, __u32)
#define LIRC_SET_TRANSMITTER_MASK      _IOW('i', 0x00000017, __u32)

typedef struct {
   int flag;
   int fd;
} IR_FD_T;

static IR_FD_T sg_ir_fd[IR_ID_MAX] = {0};

static OPERATE_RET __tkl_ir_id2fd(TY_IR_ID_E id, PINT_T fd)
{
    IR_FD_T *ifd;
    OPERATE_RET  iret = OPRT_COM_ERROR;

    if(id  >=  IR_ID_MAX ||  fd == NULL)  {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    ifd = &sg_ir_fd[(int)id];

    if(ifd->flag) {
        *fd = (INT_T)ifd->fd; 
	iret = OPRT_OK;
    }

    return iret;
}

static OPERATE_RET __tkl_ir_get_devname(TY_IR_ID_E id, PCHAR_T dev_name, INT_T len)
{
   if(id >= IR_ID_MAX || dev_name == NULL || len == 0) {
        LOGE(TAG, "%s:invalid argument\n", __func__);
        return OPRT_INVALID_PARM;
   }

   memset(dev_name, 0, len);
   snprintf(dev_name, len, IR_LIRC, (int)id);

   return OPRT_OK;
}

static OPERATE_RET __tkl_ir_init_common(TY_IR_ID_E id)
{
    int fd = -1;
    char dev_name[32] = {0};

    if (id >= IR_ID_MAX) {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    if(__tkl_ir_id2fd(id, &fd) == 0) {
        LOGE(TAG, "id:%d is inited\n", id);
        return OPRT_OK;
    }

    if(__tkl_ir_get_devname(id, (PCHAR_T)dev_name, (INT_T)32)) {
        LOGE(TAG, "id:%d get_devname fail\n", id);
        return OPRT_COM_ERROR;
    }

    fd = open(dev_name, O_RDWR);

    if (fd < 0) {
        LOGE(TAG, "open device failed errno:%d\n", -errno);
        return OPRT_COM_ERROR;
    }

    sg_ir_fd[id].fd = fd;
    sg_ir_fd[id].flag = 1;

    return OPRT_OK;
}

static void __tkl_ir_deinit_common(TY_IR_ID_E id)
{
    int fd;

    if(__tkl_ir_id2fd(id, &fd) == 0) {
        sg_ir_fd[id].flag = 0;
        sg_ir_fd[id].fd = -1;
	close (fd);
    }
}

static OPERATE_RET __tkl_ir_set_tx_carrier_common(TY_IR_ID_E id, UINT_T frequency)
{
    int fd;
    int ret;

    if(id  >=  IR_ID_MAX || frequency == 0) {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    if(__tkl_ir_id2fd(id, &fd)) {
        LOGE(TAG, "get fd fail\n");
        return OPRT_COM_ERROR;
    }

    ret = ioctl(fd, LIRC_SET_SEND_CARRIER, &frequency);
    if(ret < 0){
        LOGE(TAG, "ioctl failed errno:%d\n", -errno);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

static OPERATE_RET __tkl_ir_send_common(TY_IR_ID_E id, TY_IR_BUF_T *code) 
{
    int fd;
    int i = 0;
    unsigned int duration = 0; /* signal duration in us */
    int ret;

    if(id >= IR_ID_MAX ||  code == NULL ) {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    if(code->cnts % 2 == 0) {
        LOGE(TAG, "invalid argument, IR rawcode need paried space/pulse\n");
        return OPRT_COM_ERROR;
    }

    for(i = 0; i < code->cnts; i++) {
        if(code->buf[i] > (IR_MAX_DURATION/1000 - duration) || (code->buf[i])) {
            LOGE(TAG, "invalid argument, IR rawcode over max duration or code = 0\n");
            return OPRT_INVALID_PARM;
        }

        duration += code->buf[i];
    }

    if(__tkl_ir_id2fd(id, &fd)) {
        LOGE(TAG, "get fd fail\n");
        return OPRT_COM_ERROR;
    }

    ret = write(fd, code->buf, code->cnts*sizeof(lirc_t));

    if(ret < 0){
        LOGE(TAG, "write failed errno:%d\n", -errno);
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

static OPERATE_RET __tkl_ir_recv_common(TY_IR_ID_E id, TY_IR_BUF_T *code, INT_T ms)
{
    int fd = 0;
    int count = 0;
    fd_set fds;
    struct timeval timeout;
    struct timeval *wait = &timeout;

    if(id >= IR_ID_MAX || code == NULL || code->size == 0) {
        LOGE(TAG, "invalid argument\n");
        return OPRT_INVALID_PARM;
    }

    if(ms >= 0) {
        timeout.tv_sec  = ms/1000;
        timeout.tv_usec = (ms%1000)*1000;
    }else {
        wait =  NULL; 
    }

    if (__tkl_ir_id2fd(id, &fd)) {
        LOGE(TAG, "%s:get fd fail\n");
        return OPRT_COM_ERROR;
    }

    do{
        int ret = 0;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(fd + 1, &fds, NULL, NULL, wait);

        if(ret == 0) {
            code->cnts = count;
            return OPRT_TIMEOUT;
        }

        if(ret < 0) {
            LOGE(TAG, "select failed errno:%d\n", -errno);
            return OPRT_COM_ERROR;
        }


        /*LIRC device need a loop to read data, one interrupt one read*/
        ret = read(fd, &(code->buf[count]), sizeof(lirc_t));

        if(ret < 0) {
            LOGE(TAG, "read failed errno:%d\n", -errno);
            return OPRT_COM_ERROR;
        }

        if(ret != sizeof(lirc_t)) {
            break;
	}

        /*LIRC device will return a timeout when no IR signal receive*/
        if(LIRC_IS_TIMEOUT(code->buf[count])) {
            code->buf[count] = LIRC_VALUE(code->buf[count]);
            count++;
            break;
        }
        /*invalid signal*/
        if(LIRC_VALUE(code->buf[count]) >= 130*1000) {
            continue;
	}

        /*store the time value in buf*/
        code->buf[count] = LIRC_VALUE(code->buf[count]);

        count++;
    }while(count < code->size);

    code->cnts = count;

    return OPRT_OK;
}

OPERATE_RET tkl_ir_init(TY_IR_ID_E id)
{
    return __tkl_ir_init_common(id);
}

void tkl_ir_deinit(TY_IR_ID_E id)
{
    return __tkl_ir_deinit_common(id);
}


OPERATE_RET tkl_ir_set_tx_carrier(TY_IR_ID_E id, UINT_T frequency)
{
    return __tkl_ir_set_tx_carrier_common(id, frequency);
}

OPERATE_RET tkl_ir_send(TY_IR_ID_E id, TY_IR_BUF_T *code) 
{
    return __tkl_ir_send_common(id, code);
}

OPERATE_RET tkl_ir_recv(TY_IR_ID_E id, TY_IR_BUF_T *code, INT_T ms)
{
    return __tkl_ir_recv_common(id, code, ms);
}
