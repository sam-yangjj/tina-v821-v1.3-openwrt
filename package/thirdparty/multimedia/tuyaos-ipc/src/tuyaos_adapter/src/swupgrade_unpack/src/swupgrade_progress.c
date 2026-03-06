/*
 * (C) Copyright 2020
 * Cheng Lei, Tuya Software Engineering, chenglei@tuya.com.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "bsdqueue.h"
#include "notifier.h"
#include "network_ipc.h"
#include "swupgrade_network.h"
#include "swupgrade_progress.h"

struct progress_conn {
	SIMPLEQ_ENTRY(progress_conn) next;
	int sockfd;
};

SIMPLEQ_HEAD(connections, progress_conn);

struct swupgrade_progress {
	struct progress_msg msg;
	struct connections conns;
	pthread_mutex_t lock;
};

static struct swupgrade_progress progress;

static void send_progress_msg(void)
{
	struct progress_conn *conn, *tmp;
	struct swupgrade_progress *prbar = &progress;
	void *buf;
	size_t count;
	ssize_t n;

	SIMPLEQ_FOREACH_SAFE(conn, &prbar->conns, next, tmp) {
		buf = &prbar->msg;
		count = sizeof(prbar->msg);
		while (count > 0) {
			n = send(conn->sockfd, buf, count, MSG_NOSIGNAL);
			if (n <= 0) {
				TRACE("A progress client disappeared, removing it.");
				close(conn->sockfd);
				SIMPLEQ_REMOVE(&prbar->conns, conn,
					       	progress_conn, next);
				free(conn);
				break;
			}
			count -= (size_t)n;
			buf = (char*)buf + n;
		}
	}
}

void swupgrade_progress_init()
{
	struct swupgrade_progress *prbar = &progress;

	pthread_mutex_lock(&prbar->lock);
	prbar->msg.status = START;
	prbar->msg.cur_percent = 0;
	send_progress_msg();
	pthread_mutex_unlock(&prbar->lock);
}

void swupgrade_progress_update(unsigned int perc)
{
	struct swupgrade_progress *prbar = &progress;

	pthread_mutex_lock(&prbar->lock);
	if (perc != prbar->msg.cur_percent) {
		prbar->msg.cur_percent = perc;
		prbar->msg.status = RUN;
		send_progress_msg();
	}
	pthread_mutex_unlock(&prbar->lock);
}

void swupgrade_progress_end(RECOVERY_STATUS status)
{
	struct swupgrade_progress *prbar = &progress;

	pthread_mutex_lock(&prbar->lock);
	prbar->msg.status = status;
	send_progress_msg();
	pthread_mutex_unlock(&prbar->lock);
}

static void unlink_socket(void)
{
	unlink(get_prog_socket());
}

void *progress_bar_thread(void __attribute__ ((__unused__)) *data)
{
	int listen, connfd;
	socklen_t clilen;
	struct sockaddr_un cliaddr;
	struct swupgrade_progress *prbar = &progress;
	struct progress_conn *conn;

	pthread_mutex_init(&prbar->lock, NULL);
	SIMPLEQ_INIT(&prbar->conns);

	/* Initialize and bind to UDS */
	listen = listener_create(get_prog_socket(), SOCK_STREAM);
	if (listen < 0) {
		ERROR("Error creating IPC socket %s, exiting.", get_prog_socket());
		return NULL;
	}

	if (atexit(unlink_socket) != 0) {
		TRACE("Cannot setup socket cleanup on exit, %s won't be unlinked.",
			get_prog_socket());
	}

	do {
		clilen = sizeof(cliaddr);
		if ((connfd = accept(listen, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;
			else {
				TRACE("Accept returns: %s", strerror(errno));
				continue;
			}
		}

		/* Save the new connection to be handled by the progress thread */
		conn = (struct progress_conn *)calloc(1, sizeof(*conn));
		if (!conn) {
			ERROR("out of memory, skipping...");
			continue;
		}
		conn->sockfd = connfd;
		pthread_mutex_lock(&prbar->lock);
		SIMPLEQ_INSERT_TAIL(&prbar->conns, conn, next);
		pthread_mutex_unlock(&prbar->lock);
	} while(1);
}
