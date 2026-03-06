/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#ifndef _NETWORK_IPC_H
#define _NETWORK_IPC_H

#include <stdbool.h>
#include "progress_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_MAGIC		0x14052001

typedef enum {
	REQ_INSTALL,
	ACK,
	NACK,
	GET_STATUS,
} msgtype;

typedef union {
	char msg[128];
	struct {
		int current;
		int last_result;
		int error;
	} status;
} msgdata;

typedef struct {
	int magic;	/* magic number */
	int type;
	msgdata data;
} ipc_message;

char *get_ctrl_socket(void);

typedef int (*getstatus)(ipc_message *msg);

void ipc_end(int connfd);

/**
 * @brief prepare upgrade by ipc
 * @return success return 0, failed return -errno
 */
int swupgrade_ipc_open();

/**
 * @brief feed ota data to upgrade program
 * @param buf ota buffer pointer
 * @param size length of the buffer
 * @return success return 0, failed return -errno
 */
int swupgrade_ipc_write(char *buf, int size);

/**
 * @brief stop upgrade or release when done
 * @return success return 0, failed return -errno
 */
int swupgrade_ipc_close();

/**
 * @brief this function will block wait untill done or error happens
 * @param callback ota status will be reported by this function
 * @return success return 0, failed return -errno
 */
int ipc_wait_for_complete(getstatus callback);

/**
 * @brief get upgrade status in non-block mode
 * @return success return 0, failed return -errno
 */
int ipc_get_status(ipc_message *msg);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif
