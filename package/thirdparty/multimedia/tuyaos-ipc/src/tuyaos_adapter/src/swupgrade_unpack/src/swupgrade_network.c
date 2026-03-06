/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "bsdqueue.h"
#include "progress_ipc.h"
#include "network_ipc.h"
#include "swupgrade_network.h"
#include "swupgrade_progress.h"
#include "notifier.h"
#include "tuya_unpack.h"

#define LISTENQ				1024
#define BUFF_SIZE	 		16384

struct installer {
	int fd;
	RECOVERY_STATUS status;
	RECOVERY_STATUS last_install;
};

static struct installer inst;

static pthread_t network_thread_id;

pthread_mutex_t stream_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stream_wkup = PTHREAD_COND_INITIALIZER;

int listener_create(const char *path, int type)
{
	struct sockaddr_un servaddr;
	int listenfd = -1;

	if (listenfd == -1) {
		TRACE("creating socket at %s", path);
		listenfd = socket(AF_LOCAL, type, 0);
		unlink(path);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sun_family = AF_LOCAL;
		strcpy(servaddr.sun_path, path);

		if (bind(listenfd, (struct sockaddr *) &servaddr,
					sizeof(servaddr)) < 0) {
			close(listenfd);
			return -1;
		}

		chmod(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	}

	if (type == SOCK_STREAM) {
		if (listen(listenfd, LISTENQ) < 0) {
			close(listenfd);
			return -1;
		}
	}

	return listenfd;
}

static void unlink_socket(void)
{
	unlink(get_ctrl_socket());
}

void *network_thread(void *data)
{
	struct installer *instp = (struct installer *)data;
	int ctrllisten, ctrlconnfd;
	socklen_t clilen;
	struct sockaddr_un cliaddr;
	ipc_message msg;
	int nread;
	int ret;

	/* Initialize and bind to UDS */
	ctrllisten = listener_create(get_ctrl_socket(), SOCK_STREAM);
	if (ctrllisten < 0) {
		TRACE("Error creating IPC sockets");
		return NULL;
	}

	if (atexit(unlink_socket) != 0) {
		TRACE("Cannot setup socket cleanup on exit, %s won't be unlinked.",
			  get_ctrl_socket());
	}

	do {
		clilen = sizeof(cliaddr);
		if ((ctrlconnfd = accept(ctrllisten, (struct sockaddr *) &cliaddr,
						&clilen)) < 0) {
			if (errno == EINTR)
				continue;
			else {
				TRACE("Accept returns: %s", strerror(errno));
				continue;
			}
		}

		nread = read(ctrlconnfd, (void *)&msg, sizeof(msg));
		if (nread != sizeof(msg)) {
			TRACE("IPC message too short: fragmentation not supported");
			close(ctrlconnfd);
			continue;
		}

		pthread_mutex_lock(&stream_mutex);
		if (msg.magic == IPC_MAGIC)  {
			switch (msg.type) {
			case REQ_INSTALL:
				TRACE("Incoming network request: processing...");
				if (instp->status == IDLE) {
					instp->fd = ctrlconnfd;
					msg.type = ACK;
					/* Wake-up the installer */
					pthread_cond_signal(&stream_wkup);
				} else {
					msg.type = NACK;
					sprintf(msg.data.msg, "Installation in progress");
				}
				break;
			case GET_STATUS:
				msg.type = GET_STATUS;
				memset(msg.data.msg, 0, sizeof(msg.data.msg));
				msg.data.status.current = instp->status;
				msg.data.status.last_result = instp->last_install;
				break;
			default:
				msg.type = NACK;
			}
		} else {
			/* Wrong request */
			msg.type = NACK;
			sprintf(msg.data.msg, "Wrong request: aborting");
		}

		ret = write(ctrlconnfd, &msg, sizeof(msg));
		if (ret < 0)
			ERROR("Error write on socket ctrl");

		if (msg.type != ACK)
			close(ctrlconnfd);

		pthread_mutex_unlock(&stream_mutex);

	} while (1);

	return (void *)0; 
}

static pthread_t start_thread(void *(* start_routine) (void *), void *arg)
{
        int ret;
        pthread_t id;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        ret = pthread_create(&id, &attr, start_routine, arg);
        if (ret) {
                exit(1);
        }
        return id;
}

static int network_progress(int percentage, void *user_data)
{
	swupgrade_progress_update(percentage);

	if (percentage == 100)
		inst.last_install = SUCCESS;
	if (percentage < 0)
		inst.last_install = FAILURE;

	return 0;
}

void *network_initializer(void *data)
{
	int ret;
	ota_stream *obj;
	char buf[BUFF_SIZE] = {0};
	int len = 0;

	inst.fd = -1;
	inst.status = IDLE;

	network_thread_id = start_thread(network_thread, &inst);

	/* handle installation requests */
	while (1) {
		TRACE("Main loop Daemon");
		/* wait for someone to issue an install request */
		pthread_mutex_lock(&stream_mutex);
		pthread_cond_wait(&stream_wkup, &stream_mutex);
		pthread_mutex_unlock(&stream_mutex);
		swupgrade_progress_init();
		obj = device_upgrade_open(0, network_progress, &inst);
		inst.status = RUN;
		inst.last_install = IDLE;
		while (1) {
			len = read(inst.fd, buf, BUFF_SIZE);
			if (len <= 0) {
				TRACE("read fd(%d) ret %d %s", inst.fd, len,
						strerror(errno));
				break;
			}
			ret = device_upgrade_feed(obj, buf, len);
			if (ret)
				break;
		}

		while (inst.last_install != SUCCESS && inst.last_install != FAILURE) {
			sleep(2);
			TRACE("waitting upgrade thread done");
		}

		device_upgrade_close(obj);
		swupgrade_progress_end(inst.last_install);
		close(inst.fd);
		inst.status = IDLE;
	}

	pthread_exit((void *)0);
}
