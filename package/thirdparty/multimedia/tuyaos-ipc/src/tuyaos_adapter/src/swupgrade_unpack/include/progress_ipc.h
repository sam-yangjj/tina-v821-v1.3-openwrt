/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#ifndef _PROGRESS_IPC_H
#define _PROGRESS_IPC_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	IDLE,
	START,
	RUN,
	SUCCESS,
	FAILURE,
} RECOVERY_STATUS;

extern char* SOCKET_PROGRESS_PATH;

/*
 * Message sent via progress socket.
 * Data is sent in LE if required.
 */
struct progress_msg {
	unsigned int	magic;		/* Magic Number */
	RECOVERY_STATUS	status;		/* Update Status (Running, Failure) */
	unsigned int	cur_percent;	/* % in current step */
};

char *get_prog_socket(void);

/* Standard function to connect to progress interface */
int progress_ipc_connect(bool reconnect);

/*
 * In case more as an instance of SWUpdate is running, this allows to select
 * which should be taken
 */
int progress_ipc_connect_with_path(const char *socketpath, bool reconnect);

/* Retrieve messages from progress interface (it blocks) */
int progress_ipc_receive(int *connfd, struct progress_msg *msg);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif
