/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "progress_ipc.h"
#include "network_ipc.h"

static char* SOCKET_CTRL_PATH = NULL;

#define SOCKET_CTRL_DEFAULT  "sockinstctrl"

struct async_lib {
	int connfd;
	int status;
	getstatus get;
};

static struct async_lib request;

#define get_request()	(&request)

char *get_ctrl_socket(void)
{
	if (!SOCKET_CTRL_PATH || !strlen(SOCKET_CTRL_PATH)) {
		const char *tmpdir = getenv("TMPDIR");
		if (!tmpdir)
			tmpdir = "/tmp";

		if (asprintf(&SOCKET_CTRL_PATH, "%s/%s", tmpdir,
					SOCKET_CTRL_DEFAULT) == -1)
			return (char *)"/tmp/"SOCKET_CTRL_DEFAULT;
	}

	return SOCKET_CTRL_PATH;
}

static int prepare_ipc(void)
{
	int connfd;
	int ret;

	struct sockaddr_un servaddr;

	connfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;

	strncpy(servaddr.sun_path, get_ctrl_socket(), sizeof(servaddr.sun_path));

	ret = connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if (ret < 0) {
		close(connfd);
		return ret;
	}

	return connfd;
}

static int __ipc_get_status(int connfd, ipc_message *msg)
{
	ssize_t ret;

	memset(msg, 0, sizeof(*msg));
	msg->magic = IPC_MAGIC;
	msg->type = GET_STATUS;
	ret = write(connfd, msg, sizeof(*msg));
	if (ret != sizeof(*msg))
		return -1;

	ret = read(connfd, msg, sizeof(*msg));
	if (ret <= 0)
		return -1;

	return 0;
}

int ipc_get_status(ipc_message *msg)
{
	int ret;
	int connfd;

	connfd = prepare_ipc();
	if (connfd < 0) {
		return -1;
	}
	ret = __ipc_get_status(connfd, msg);
	close(connfd);

	return ret;
}

int ipc_wait_for_complete(getstatus callback)
{
	int fd;
	RECOVERY_STATUS status = IDLE;
	ipc_message message;
	int ret;

	do {
		fd = prepare_ipc();
		if (fd < 0)
			break;
		ret = __ipc_get_status(fd, &message);
		ipc_end(fd);

		if (ret < 0) {
			printf("__ipc_get_status returned %d\n", ret);
			message.data.status.last_result = FAILURE;
			break;
		}

		if (status != (RECOVERY_STATUS)message.data.status.current) {
			if (callback)
				callback(&message);
		} else {
			sleep(1);
		}

		status = (RECOVERY_STATUS)message.data.status.current;

	} while (message.data.status.current != IDLE);

	return message.data.status.last_result;
}

int ipc_inst_start_ext()
{
	int connfd;
	ipc_message msg;
	ssize_t ret;

	connfd = prepare_ipc();
	if (connfd < 0)
		return -1;

	memset(&msg, 0, sizeof(msg));

	msg.magic = IPC_MAGIC;
	msg.type = REQ_INSTALL;

	ret = write(connfd, &msg, sizeof(msg));
	if (ret != sizeof(msg)) {
		close(connfd);
		return -1;
	}

	ret = read(connfd, &msg, sizeof(msg));
	if (ret <= 0) {
		close(connfd);
		return -1;
	}

	if (msg.type != ACK) {
		close(connfd);
		return -1;
	}

	return connfd;
}

int ipc_send_data(int connfd, char *buf, int size)
{
	ssize_t ret;

	ret = write(connfd, buf, (size_t)size);
	if (ret != size) {
		return -1;
	}

	return (int)ret;
}

void ipc_end(int connfd)
{
	if (connfd <= 0)
		return;

	close(connfd);
}

int swupgrade_ipc_open()
{
	struct async_lib *rq;
	int connfd;

	rq = get_request();

	connfd = ipc_inst_start_ext();
	if (connfd < 0)
		return connfd;

	rq->connfd = connfd;

	return 0;
}

int swupgrade_ipc_write(char *buf, int size)
{
	int ret;
	struct async_lib *rq;

	rq = get_request();

	if (!buf || size <= 0) {
		ipc_end(rq->connfd);
		rq->connfd = -1;
		return -1;
	}

	ret = ipc_send_data(rq->connfd, buf, size);

	return ret;
}

int swupgrade_ipc_close()
{
	struct async_lib *rq;

	rq = get_request();

	ipc_end(rq->connfd);

	return -1;
}
